#pragma once

#include <imf/runtime/cpu/CpuTexture.hpp>

#include <imf/core/glm.hpp>
#include <imf/core/Region.hpp>
#include <imf/core/ThreadPool.hpp>

#include <stdexcept>
#include <utility>

namespace imf::runtime::cpu
{

class Rasterizer
{
public:
	//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
	//MSAA x4
	constexpr static glm::vec2 kSamples[] =
	{
		glm::vec2(-2.0f / 16.0f, -6.0f / 16.0f),
		glm::vec2(6.0f / 16.0f, -2.0f / 16.0f),
		glm::vec2(-6.0f / 16.0f, 2.0f / 16.0f),
		glm::vec2(2.0f / 16.0f, 6.0f / 16.0f),
	};

	template<typename T, glm::qualifier Q>
	static float edgeFunc(const glm::vec<2, T, Q>& x, const glm::vec<2, T, Q>& y) noexcept
	{
		//the same as `glm::cross(x,y).z`;
		return x.x * y.y - y.x * x.y;
	};

	template<typename T, glm::qualifier Q>
	static glm::vec<4, T, Q> barycentric(const glm::vec<2, T, Q>& a, const glm::vec<2, T, Q>& b, const glm::vec<2, T, Q>& c, const glm::vec<2, T, Q>& point) noexcept
	{
		const auto Sa = edgeFunc(c - b, point - b);
		const auto Sb = edgeFunc(a - c, point - c);
		const auto Sc = edgeFunc(b - a, point - a);

		const auto S = Sa + Sb + Sc;
		return
		{
			Sa,
			Sb,
			Sc,
			S
		};
	}

	template<typename Callable>
	static auto rasterize
	(
		core::ThreadPool& pool,
		CpuTexture& target,
		const glm::vec2& origin,
		const core::Region::Triangulation& triangulation,
		const glm::mat3& localToWorldMat,
		const Callable& callback
	) -> decltype(callback(std::declval<glm::mat4x2>(), std::declval<int&>()), void())
	{
		assert(target.dim().z == 1);
		if (target.dim().z != 1)
		{
			throw std::invalid_argument("only 2D target is supported");
		}

		const auto& msaaLevel = target.getMsaaLevel();
		const auto msaaDim = glm::ivec2(msaaLevel.dim.xy());
		const auto msaaSize = msaaLevel.size;
		const auto msaaData = msaaLevel.storage.get();
		const auto readTransformFunc = core::get_convert_func(target.format(), core::TextureFormat::RGBA32F);
		const auto writeTransformFunc = core::get_convert_func(core::TextureFormat::RGBA32F, target.format());

		pool.forEachSync([&](int from, int to)
		{
			// 2 rows per original image * 2 rows per 4xMSAA = 4 msaa rows in total 
			auto msaaRowBuffer = std::vector<glm::vec4>(msaaDim.x * 4);

			for (int quadRowIdx = from; quadRowIdx < to; quadRowIdx++)
			{
				const auto msaaRowIdx0 = quadRowIdx * 4;
				const auto msaaRowIdx1 = msaaRowIdx0 + 1;
				const auto msaaRowIdx2 = msaaRowIdx0 + 2;
				const auto msaaRowIdx3 = msaaRowIdx0 + 3;

				auto msaaRowPtr0 = msaaData + msaaRowIdx0 * msaaSize.rowByteSize;
				auto msaaRowPtr1 = msaaData + msaaRowIdx1 * msaaSize.rowByteSize;
				auto msaaRowPtr2 = msaaData + msaaRowIdx2 * msaaSize.rowByteSize;
				auto msaaRowPtr3 = msaaData + msaaRowIdx3 * msaaSize.rowByteSize;

				readTransformFunc(msaaRowPtr0, msaaRowBuffer.data() + msaaDim.x * 0, msaaDim.x);
				readTransformFunc(msaaRowPtr1, msaaRowBuffer.data() + msaaDim.x * 1, msaaDim.x);

				if (msaaRowIdx2 < msaaDim.y)
				{
					readTransformFunc(msaaRowPtr2, msaaRowBuffer.data() + msaaDim.x * 2, msaaDim.x);
					readTransformFunc(msaaRowPtr3, msaaRowBuffer.data() + msaaDim.x * 3, msaaDim.x); // guaranteed to be in range
				}

				for (const auto& triangle : triangulation.indices)
				{
					const auto v0 = core::projectToPlane(localToWorldMat, triangulation.vertices[triangle.x]);
					const auto v1 = core::projectToPlane(localToWorldMat, triangulation.vertices[triangle.y]);
					const auto v2 = core::projectToPlane(localToWorldMat, triangulation.vertices[triangle.z]);

					const auto targetDim = msaaDim / 2;
					const auto rowIdx1 = msaaRowIdx0 / 2;
					const auto rowIdx2 = msaaRowIdx2 / 2;

					for (int colIdx = 0; colIdx < targetDim.x; colIdx += 2)
					{
						const auto pixelPosQuad = glm::mat4x2
						{
							origin + glm::vec2(colIdx + 0.5f, (targetDim.y - 1) - (rowIdx2 + 0.5f)),
							origin + glm::vec2(colIdx + 1.5f, (targetDim.y - 1) - (rowIdx2 + 0.5f)),
							origin + glm::vec2(colIdx + 0.5f, (targetDim.y - 1) - (rowIdx1 + 0.5f)),
							origin + glm::vec2(colIdx + 1.5f, (targetDim.y - 1) - (rowIdx1 + 0.5f))
						};

						// 4xMSAA * 4 pixels in quad = 16 samples in total
						// 'int' is at least 16 bit
						int coverageMask = 0;

						for (int pixelIdx = 0; pixelIdx != 4; ++pixelIdx)
						{
							const auto& pixelPos = pixelPosQuad[pixelIdx];

							for (int subsampleIdx = 0; subsampleIdx != 4; ++subsampleIdx)
							{
								const auto& subsample = kSamples[subsampleIdx];

								const auto areas = barycentric(v0, v1, v2, pixelPos + subsample);
								const auto normalizedAreas = glm::vec3(areas) / areas.w;

								const auto maskBit = static_cast<int>(glm::compMin(normalizedAreas) >= 0.0f);
								coverageMask |= maskBit << (pixelIdx * 4 + subsampleIdx);
							}
						}

						// if entire quad is not covered
						if (coverageMask == 0)
						{
							continue;
						}

						auto pixelQuadColor = callback
						(
							pixelPosQuad,
							coverageMask
						);

						const auto hasRight = colIdx + 1 < targetDim.x;
						const auto hasBottom = rowIdx2 < targetDim.y;

						// pixel quad layout:
						// 2 3
						// 0 1

						// MSAA samples layout:
						// 10 11 | 14 15
						//  8  9 | 12 13
						// ------+------
						//  2  3 |  6  7
						//  0  1 |  4  5

						// first msaa row
						if (coverageMask & (1 << 10)) msaaRowBuffer[colIdx * 2] = pixelQuadColor[2];
						if (coverageMask & (1 << 11)) msaaRowBuffer[colIdx * 2 + 1] = pixelQuadColor[2];
						if (hasRight)
						{
							if ((coverageMask & (1 << 14))) msaaRowBuffer[(colIdx + 1) * 2] = pixelQuadColor[3];
							if ((coverageMask & (1 << 15))) msaaRowBuffer[(colIdx + 1) * 2 + 1] = pixelQuadColor[3];
						}

						// second msaa row
						if (coverageMask & (1 << 8)) msaaRowBuffer[msaaDim.x + colIdx * 2] = pixelQuadColor[2];
						if (coverageMask & (1 << 9)) msaaRowBuffer[msaaDim.x + colIdx * 2 + 1] = pixelQuadColor[2];
						if (hasRight)
						{
							if ((coverageMask & (1 << 12))) msaaRowBuffer[msaaDim.x + (colIdx + 1) * 2] = pixelQuadColor[3];
							if ((coverageMask & (1 << 13))) msaaRowBuffer[msaaDim.x + (colIdx + 1) * 2 + 1] = pixelQuadColor[3];
						}

						if (hasBottom)
						{
							// third msaa row
							if ((coverageMask & (1 << 2))) msaaRowBuffer[msaaDim.x * 2 + colIdx * 2] = pixelQuadColor[0];
							if ((coverageMask & (1 << 3))) msaaRowBuffer[msaaDim.x * 2 + colIdx * 2 + 1] = pixelQuadColor[0];
							if (hasRight)
							{
								if ((coverageMask & (1 << 6))) msaaRowBuffer[msaaDim.x * 2 + (colIdx + 1) * 2] = pixelQuadColor[1];
								if ((coverageMask & (1 << 7))) msaaRowBuffer[msaaDim.x * 2 + (colIdx + 1) * 2 + 1] = pixelQuadColor[1];
							}

							// forth msaa row
							if ((coverageMask & (1 << 0))) msaaRowBuffer[msaaDim.x * 3 + colIdx * 2] = pixelQuadColor[0];
							if ((coverageMask & (1 << 1))) msaaRowBuffer[msaaDim.x * 3 + colIdx * 2 + 1] = pixelQuadColor[0];
							if (hasRight)
							{
								if ((coverageMask & (1 << 4))) msaaRowBuffer[msaaDim.x * 3 + (colIdx + 1) * 2] = pixelQuadColor[1];
								if ((coverageMask & (1 << 5))) msaaRowBuffer[msaaDim.x * 3 + (colIdx + 1) * 2 + 1] = pixelQuadColor[1];
							}
						}
					}
				}

				writeTransformFunc(msaaRowBuffer.data(), msaaRowPtr0, msaaDim.x);
				writeTransformFunc(msaaRowBuffer.data() + msaaDim.x * 1, msaaRowPtr1, msaaDim.x);

				if (msaaRowIdx2 < msaaDim.y)
				{
					writeTransformFunc(msaaRowBuffer.data() + msaaDim.x * 2, msaaRowPtr2, msaaDim.x);
					writeTransformFunc(msaaRowBuffer.data() + msaaDim.x * 3, msaaRowPtr3, msaaDim.x); // guaranteed to be in range
				}
			}
		}, (msaaDim.y + 4 - 1) / 4); // i.e. (17|18|19|20 rows + 3) / 4 = 5 batches
	}

	template<typename Callable>
	static auto rasterize
	(
		core::ThreadPool& pool,
		CpuTexture& target,
		const glm::vec2& origin,
		const core::Region::Triangulation& triangulation,
		const glm::mat3& localToWorldMat,
		const Callable& callback
	) -> decltype(callback(std::declval<glm::mat4x2>()), void())
	{
		rasterize(pool, target, origin, triangulation, localToWorldMat, [&](const glm::mat4x2& pixelQuad, int& /*unused coverage mask*/)
		{
			return callback(pixelQuad);
		});
	}
};

}
