#include <iostream>
#include <fstream>
#include <cassert>
#include <fbxsdk.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <Windows.h>
#include <string>
#include <random>
#include <direct.h>
#include <thread>
#include "Math.h"
#include "xxhash.h"
#include "mesh_optimizer/meshoptimizer.h"
using namespace fbxsdk;
using namespace DirectX;

using uint32 = std::uint32_t;
using uint16 = std::uint16_t;
using uint8 = std::uint8_t;

template <typename T>
using VectorArray = std::vector<T>;

template <typename T, typename U>
using UnorderedMap = std::unordered_map<T, U>;

template <typename T>
using UnorderedSet = std::unordered_set<T>;

using String = std::string;

constexpr char LOG_LINE[] = "=====================================================";
constexpr char LOG_COLOR_RED[] = "";
constexpr char LOG_COLOR_GREEN[] = "";
constexpr char LOG_COLOR_END[] = "";

namespace Octahedron {
	uint16 EncodeUnorm(FbxVector2 vector) {
		uint16 result = 0;
		uint8* v1 = reinterpret_cast<uint8*>(&result);
		v1[0] = static_cast<uint8>(UINT8_MAX * vector[0]);
		v1[1] = static_cast<uint8>(UINT8_MAX * vector[1]);
		return result;
	}

	FbxVector2 OctWrap(FbxVector2 v) {
		FbxVector2 signedV(v[0] >= 0.0f ? 1.0 : -1.0, v[1] >= 0.0f ? 1.0 : -1.0);
		return (FbxVector2(1.0f - abs(v[0]), 1.0f - abs(v[1]))) * signedV;
		//return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
	}

	FbxVector2 Encode(FbxVector4 n) {
		n /= (abs(n[0]) + abs(n[1]) + abs(n[2]));
		FbxVector2 xy(n[0], n[1]);
		xy = n[3] >= 0.0 ? xy : OctWrap(xy);
		xy = xy * 0.5 + 0.5;;
		//n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
		//n.xy = n.xy * 0.5 + 0.5;
		return xy;
	}
}

uint32 packNormalAndTangentOctahedron(Float3 normal, Float3 tangent) {
	FbxVector2 valueNormal = Octahedron::Encode(FbxVector4(normal.x, normal.y, normal.z));
	FbxVector2 valueTangent = Octahedron::Encode(FbxVector4(tangent.x, tangent.y, tangent.z));
	uint32 result = 0;
	uint16* v1 = reinterpret_cast<uint16*>(&result);
	v1[0] = Octahedron::EncodeUnorm(valueNormal);
	v1[1] = Octahedron::EncodeUnorm(valueTangent);
	return result;
}

namespace {
	struct EdgeEntry {
		uint32   i0;
		uint32   i1;
		uint32   i2;

		uint32   Face;
		EdgeEntry* Next;
	};

	union PackedTriangle {
		struct {
			uint32 i0 : 8;
			uint32 i1 : 8;
			uint32 i2 : 8;
			uint32 _unused : 8;
		} indices;
		uint32 packed;
	};

	struct InlineMeshlet {
		struct PackedTriangle {
			uint32 i0 : 8;
			uint32 i1 : 8;
			uint32 i2 : 8;
			uint32 spare : 8;
		};

		std::vector<uint32>         UniqueVertexIndices;
		std::vector<PackedTriangle> PrimitiveIndices;
	};

	enum Flags : uint32_t {
		CNORM_DEFAULT = 0x0,
		CNORM_WIND_CW = 0x4
	};

	struct Subset {
		uint32_t Offset;
		uint32_t Count;
	};

	struct Meshlet {
		uint32_t VertCount;
		uint32_t VertOffset;
		uint32_t PrimCount;
		uint32_t PrimOffset;
	};

	struct CullData {
		DirectX::XMFLOAT3 BoundingBoxMin; // xyz = corner
		DirectX::XMFLOAT3 BoundingBoxMax; // xyz = corner
		uint8_t           NormalCone[4];  // xyz = axis, w = sin(a + 90)
		float             ApexOffset;     // apex = center - axis * offset
	};

	inline XMVECTOR QuantizeSNorm(XMVECTOR value) {
		return (XMVectorClamp(value, g_XMNegativeOne, g_XMOne) * 0.5f + XMVectorReplicate(0.5f)) * 255.0f;
	}

	inline XMVECTOR QuantizeUNorm(XMVECTOR value) {
		return (XMVectorClamp(value, g_XMZero, g_XMOne)) * 255.0f;
	}

	// Determines whether a candidate triangle can be added to a specific meshlet; if it can, does so.
	bool AddToMeshlet(uint32_t maxVerts, uint32_t maxPrims, InlineMeshlet& meshlet, uint32(&tri)[3]) {
		// Are we already full of vertices?
		if (meshlet.UniqueVertexIndices.size() == maxVerts)
			return false;

		// Are we full, or can we store an additional primitive?
		if (meshlet.PrimitiveIndices.size() == maxPrims)
			return false;

		static const uint32_t Undef = uint32_t(-1);
		uint32_t indices[3] = { Undef, Undef, Undef };
		uint32_t newCount = 3;

		for (uint32_t i = 0; i < meshlet.UniqueVertexIndices.size(); ++i) {
			for (uint32_t j = 0; j < 3; ++j) {
				if (meshlet.UniqueVertexIndices[i] == tri[j]) {
					indices[j] = i;
					--newCount;
				}
			}
		}

		// Will this triangle fit?
		if (meshlet.UniqueVertexIndices.size() + newCount > maxVerts)
			return false;

		// Add unique vertex indices to unique vertex index list
		for (uint32_t j = 0; j < 3; ++j) {
			if (indices[j] == Undef) {
				indices[j] = static_cast<uint32_t>(meshlet.UniqueVertexIndices.size());
				meshlet.UniqueVertexIndices.push_back(tri[j]);
			}
		}

		// Add the new primitive 
		typename InlineMeshlet::PackedTriangle prim = {};
		prim.i0 = indices[0];
		prim.i1 = indices[1];
		prim.i2 = indices[2];

		meshlet.PrimitiveIndices.push_back(prim);

		return true;
	}

	void MinMaxBoundingBox(XMFLOAT3* points, uint32_t count, XMVECTOR& outBoundsMin, XMVECTOR& outBoundsMax) {
		assert(points != nullptr && count != 0);

		XMVECTOR boundsMin = XMVectorSet(FLT_MIN, FLT_MAX, FLT_MAX, 1);
		XMVECTOR boundsMax = XMVectorSet(-FLT_MIN, -FLT_MAX, -FLT_MAX, 1);
		for (uint32 i = 0; i < count; ++i) {
			XMVECTOR point = XMLoadFloat3(&points[i]);
			boundsMin = XMVectorMin(boundsMin, point);
			boundsMax = XMVectorMax(boundsMax, point);
		}

		outBoundsMin = boundsMin;
		outBoundsMax = boundsMax;
	}

	XMVECTOR MinimumBoundingSphere(XMFLOAT3* points, uint32_t count) {
		assert(points != nullptr && count != 0);

		// Find the min & max points indices along each axis.
		uint32_t minAxis[3] = { 0, 0, 0 };
		uint32_t maxAxis[3] = { 0, 0, 0 };

		for (uint32_t i = 1; i < count; ++i) {
			float* point = (float*)(points + i);

			for (uint32_t j = 0; j < 3; ++j) {
				float* min = (float*)(&points[minAxis[j]]);
				float* max = (float*)(&points[maxAxis[j]]);

				minAxis[j] = point[j] < min[j] ? i : minAxis[j];
				maxAxis[j] = point[j] > max[j] ? i : maxAxis[j];
			}
		}

		// Find axis with maximum span.
		XMVECTOR distSqMax = g_XMZero;
		uint32_t axis = 0;

		for (uint32_t i = 0; i < 3u; ++i) {
			XMVECTOR min = XMLoadFloat3(&points[minAxis[i]]);
			XMVECTOR max = XMLoadFloat3(&points[maxAxis[i]]);

			XMVECTOR distSq = XMVector3LengthSq(max - min);
			if (XMVector3Greater(distSq, distSqMax)) {
				distSqMax = distSq;
				axis = i;
			}
		}

		// Calculate an initial starting center point & radius.
		XMVECTOR p1 = XMLoadFloat3(&points[minAxis[axis]]);
		XMVECTOR p2 = XMLoadFloat3(&points[maxAxis[axis]]);

		XMVECTOR center = (p1 + p2) * 0.5f;
		XMVECTOR radius = XMVector3Length(p2 - p1) * 0.5f;
		XMVECTOR radiusSq = radius * radius;

		// Add all our points to bounding sphere expanding radius & recalculating center point as necessary.
		for (uint32_t i = 0; i < count; ++i) {
			XMVECTOR point = XMLoadFloat3(points + i);
			XMVECTOR distSq = XMVector3LengthSq(point - center);

			if (XMVector3Greater(distSq, radiusSq)) {
				XMVECTOR dist = XMVectorSqrt(distSq);
				XMVECTOR k = (radius / dist) * 0.5f + XMVectorReplicate(0.5f);

				center = center * k + point * (g_XMOne - k);
				radius = (radius + dist) * 0.5f;
			}
		}

		// Populate a single XMVECTOR with center & radius data.
		XMVECTOR select0001 = XMVectorSelectControl(0, 0, 0, 1);
		return XMVectorSelect(center, radius, select0001);
	}

	bool IsMeshletFull(uint32_t maxVerts, uint32_t maxPrims, const InlineMeshlet& meshlet) {
		assert(meshlet.UniqueVertexIndices.size() <= maxVerts);
		assert(meshlet.PrimitiveIndices.size() <= maxPrims);

		return meshlet.UniqueVertexIndices.size() == maxVerts
			|| meshlet.PrimitiveIndices.size() == maxPrims;
	}

	// Compute number of triangle vertices already exist in the meshlet
	uint32_t ComputeReuse(const InlineMeshlet& meshlet, uint32(&triIndices)[3]) {
		uint32_t count = 0;

		for (uint32_t i = 0; i < static_cast<uint32_t>(meshlet.UniqueVertexIndices.size()); ++i) {
			for (uint32_t j = 0; j < 3u; ++j) {
				if (meshlet.UniqueVertexIndices[i] == triIndices[j]) {
					++count;
				}
			}
		}

		return count;
	}

	// Sort in reverse order to use vector as a queue with pop_back.
	bool CompareScores(const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) {
		return a.second > b.second;
	}

	XMVECTOR ComputeNormal(XMFLOAT3* tri) {
		XMVECTOR p0 = XMLoadFloat3(&tri[0]);
		XMVECTOR p1 = XMLoadFloat3(&tri[1]);
		XMVECTOR p2 = XMLoadFloat3(&tri[2]);

		XMVECTOR v01 = p0 - p1;
		XMVECTOR v02 = p0 - p2;

		return XMVector3Normalize(XMVector3Cross(v01, v02));
	}

	// Computes a candidacy score based on spatial locality, orientational coherence, and vertex re-use within a meshlet.
	float ComputeScore(const InlineMeshlet& meshlet, XMVECTOR sphere, XMVECTOR normal, uint32(&triIndices)[3], XMFLOAT3* triVerts) {
		const float reuseWeight = 0.334f;
		const float locWeight = 0.333f;
		const float oriWeight = 0.333f;

		// Vertex reuse
		uint32_t reuse = ComputeReuse(meshlet, triIndices);
		XMVECTOR reuseScore = g_XMOne - (XMVectorReplicate(float(reuse)) / 3.0f);

		// Distance from center point
		XMVECTOR maxSq = g_XMZero;
		for (uint32_t i = 0; i < 3u; ++i) {
			XMVECTOR v = sphere - XMLoadFloat3(&triVerts[i]);
			maxSq = XMVectorMax(maxSq, XMVector3Dot(v, v));
		}
		XMVECTOR r = XMVectorSplatW(sphere);
		XMVECTOR r2 = r * r;
		XMVECTOR locScore = XMVectorLog(maxSq / r2 + g_XMOne);

		// Angle between normal and meshlet cone axis
		XMVECTOR n = ComputeNormal(triVerts);
		XMVECTOR d = XMVector3Dot(n, normal);
		XMVECTOR oriScore = (-d + g_XMOne) / 2.0f;

		XMVECTOR b = reuseWeight * reuseScore + locWeight * locScore + oriWeight * oriScore;

		return XMVectorGetX(b);
	}

	size_t CRCHash(const uint32_t* dwords, uint32_t dwordCount) {
		size_t h = 0;

		for (uint32_t i = 0; i < dwordCount; ++i) {
			uint32_t highOrd = h & 0xf8000000;
			h = h << 5;
			h = h ^ (highOrd >> 27);
			h = h ^ size_t(dwords[i]);
		}

		return h;
	}

	template <typename T>
	inline size_t Hash(const T& val) {
		return std::hash<T>()(val);
	}
}

namespace std {
	template <> struct hash<XMFLOAT3> { size_t operator()(const XMFLOAT3& v) const { return CRCHash(reinterpret_cast<const uint32_t*>(&v), sizeof(v) / 4); } };
}

namespace {
	void BuildAdjacencyList(
		const uint32* indices, uint32_t indexCount,
		const XMFLOAT3* positions, uint32_t vertexCount,
		uint32_t* adjacency
	) {
		const uint32_t triCount = indexCount / 3;
		// Find point reps (unique positions) in the position stream
		// Create a mapping of non-unique vertex indices to point reps
		std::vector<uint32> pointRep;
		pointRep.resize(vertexCount);

		std::unordered_map<size_t, uint32> uniquePositionMap;
		uniquePositionMap.reserve(vertexCount);

		for (uint32_t i = 0; i < vertexCount; ++i) {
			XMFLOAT3 position = *(positions + i);
			size_t hash = Hash(position);

			auto it = uniquePositionMap.find(hash);
			if (it != uniquePositionMap.end()) {
				// Position already encountered - reference previous index
				pointRep[i] = it->second;
			}
			else {
				// New position found - add to hash table and LUT
				uniquePositionMap.insert(std::make_pair(hash, static_cast<uint32>(i)));
				pointRep[i] = static_cast<uint32>(i);
			}
		}

		// Create a linked list of edges for each vertex to determine adjacency
		const uint32_t hashSize = vertexCount / 3;

		std::unique_ptr<EdgeEntry* []> hashTable(new EdgeEntry * [hashSize]);
		std::unique_ptr<EdgeEntry[]> entries(new EdgeEntry[triCount * 3]);

		std::memset(hashTable.get(), 0, sizeof(EdgeEntry*) * hashSize);
		uint32_t entryIndex = 0;

		for (uint32_t iFace = 0; iFace < triCount; ++iFace) {
			uint32_t index = iFace * 3;

			// Create a hash entry in the hash table for each each.
			for (uint32_t iEdge = 0; iEdge < 3; ++iEdge) {
				uint32 i0 = pointRep[indices[index + (iEdge % 3)]];
				uint32 i1 = pointRep[indices[index + ((iEdge + 1) % 3)]];
				uint32 i2 = pointRep[indices[index + ((iEdge + 2) % 3)]];

				auto& entry = entries[entryIndex++];
				entry.i0 = i0;
				entry.i1 = i1;
				entry.i2 = i2;

				uint32_t key = entry.i0 % hashSize;

				entry.Next = hashTable[key];
				entry.Face = iFace;

				hashTable[key] = &entry;
			}
		}


		// Initialize the adjacency list
		std::memset(adjacency, uint32_t(-1), indexCount * sizeof(uint32_t));

		for (uint32_t iFace = 0; iFace < triCount; ++iFace) {
			uint32_t index = iFace * 3;

			for (uint32_t point = 0; point < 3; ++point) {
				if (adjacency[iFace * 3 + point] != uint32_t(-1))
					continue;

				// Look for edges directed in the opposite direction.
				uint32 i0 = pointRep[indices[index + ((point + 1) % 3)]];
				uint32 i1 = pointRep[indices[index + (point % 3)]];
				uint32 i2 = pointRep[indices[index + ((point + 2) % 3)]];

				// Find a face sharing this edge
				uint32_t key = i0 % hashSize;

				EdgeEntry* found = nullptr;
				EdgeEntry* foundPrev = nullptr;

				for (EdgeEntry* current = hashTable[key], *prev = nullptr; current != nullptr; prev = current, current = current->Next) {
					if (current->i1 == i1 && current->i0 == i0) {
						found = current;
						foundPrev = prev;
						break;
					}
				}

				// Cache this face's normal
				XMVECTOR n0;
				{
					XMVECTOR p0 = XMLoadFloat3(&positions[i1]);
					XMVECTOR p1 = XMLoadFloat3(&positions[i0]);
					XMVECTOR p2 = XMLoadFloat3(&positions[i2]);

					XMVECTOR e0 = p0 - p1;
					XMVECTOR e1 = p1 - p2;

					n0 = XMVector3Normalize(XMVector3Cross(e0, e1));
				}

				// Use face normal dot product to determine best edge-sharing candidate.
				float bestDot = -2.0f;
				for (EdgeEntry* current = found, *prev = foundPrev; current != nullptr; prev = current, current = current->Next) {
					if (bestDot == -2.0f || (current->i1 == i1 && current->i0 == i0)) {
						XMVECTOR p0 = XMLoadFloat3(&positions[current->i0]);
						XMVECTOR p1 = XMLoadFloat3(&positions[current->i1]);
						XMVECTOR p2 = XMLoadFloat3(&positions[current->i2]);

						XMVECTOR e0 = p0 - p1;
						XMVECTOR e1 = p1 - p2;

						XMVECTOR n1 = XMVector3Normalize(XMVector3Cross(e0, e1));

						float dot = XMVectorGetX(XMVector3Dot(n0, n1));

						if (dot > bestDot) {
							found = current;
							foundPrev = prev;
							bestDot = dot;
						}
					}
				}

				// Update hash table and adjacency list
				if (found && found->Face != uint32_t(-1)) {
					// Erase the found from the hash table linked list.
					if (foundPrev != nullptr) {
						foundPrev->Next = found->Next;
					}
					else {
						hashTable[key] = found->Next;
					}

					// Update adjacency information
					adjacency[iFace * 3 + point] = found->Face;

					// Search & remove this face from the table linked list
					uint32_t key2 = i1 % hashSize;

					for (EdgeEntry* current = hashTable[key2], *prev = nullptr; current != nullptr; prev = current, current = current->Next) {
						if (current->Face == iFace && current->i1 == i0 && current->i0 == i1) {
							if (prev != nullptr) {
								prev->Next = current->Next;
							}
							else {
								hashTable[key2] = current->Next;
							}

							break;
						}
					}

					bool linked = false;
					for (uint32_t point2 = 0; point2 < point; ++point2) {
						if (found->Face == adjacency[iFace * 3 + point2]) {
							linked = true;
							adjacency[iFace * 3 + point] = uint32_t(-1);
							break;
						}
					}

					if (!linked) {
						uint32_t edge2 = 0;
						for (; edge2 < 3; ++edge2) {
							uint32 k = indices[found->Face * 3 + edge2];
							if (k == uint32_t(-1))
								continue;

							if (pointRep[k] == i0)
								break;
						}

						if (edge2 < 3) {
							adjacency[found->Face * 3 + edge2] = iFace;
						}
					}
				}
			}
		}
	}

	void Meshletize(
		uint32_t maxVerts, uint32_t maxPrims,
		const uint32* indices, uint32_t indexCount,
		const XMFLOAT3* positions, uint32_t vertexCount,
		std::vector<InlineMeshlet>& output
	) {
		const uint32_t triCount = indexCount / 3;

		// Build a primitive adjacency list
		std::vector<uint32_t> adjacency;
		adjacency.resize(indexCount);

		BuildAdjacencyList(indices, indexCount, positions, vertexCount, adjacency.data());

		// Rest our outputs
		output.clear();
		output.emplace_back();
		auto* curr = &output.back();

		// Bitmask of all triangles in mesh to determine whether a specific one has been added.
		std::vector<bool> checklist;
		checklist.resize(triCount);

		std::vector<XMFLOAT3> m_positions;
		std::vector<XMFLOAT3> normals;
		std::vector<std::pair<uint32_t, float>> candidates;
		std::unordered_set<uint32_t> candidateCheck;

		XMVECTOR psphere, normal;

		// Arbitrarily start at triangle zero.
		uint32_t triIndex = 0;
		candidates.push_back(std::make_pair(triIndex, 0.0f));
		candidateCheck.insert(triIndex);

		// Continue adding triangles until 
		while (!candidates.empty()) {
			uint32_t index = candidates.back().first;
			candidates.pop_back();

			uint32 tri[3] =
			{
				indices[index * 3],
				indices[index * 3 + 1],
				indices[index * 3 + 2],
			};

			assert(tri[0] < vertexCount);
			assert(tri[1] < vertexCount);
			assert(tri[2] < vertexCount);

			// Try to add triangle to meshlet
			if (AddToMeshlet(maxVerts, maxPrims, *curr, tri)) {
				// Success! Mark as added.
				checklist[index] = true;

				// Add m_positions & normal to list
				XMFLOAT3 points[3] =
				{
					positions[tri[0]],
					positions[tri[1]],
					positions[tri[2]],
				};

				m_positions.push_back(points[0]);
				m_positions.push_back(points[1]);
				m_positions.push_back(points[2]);

				XMFLOAT3 Normal;
				XMStoreFloat3(&Normal, ComputeNormal(points));
				normals.push_back(Normal);

				// Compute new bounding sphere & normal axis
				psphere = MinimumBoundingSphere(m_positions.data(), static_cast<uint32_t>(m_positions.size()));

				XMVECTOR nsphere = MinimumBoundingSphere(normals.data(), static_cast<uint32_t>(normals.size()));
				normal = XMVector3Normalize(nsphere);

				// Find and add all applicable adjacent triangles to candidate list
				const uint32_t adjIndex = index * 3;

				uint32_t adj[3] =
				{
					adjacency[adjIndex],
					adjacency[adjIndex + 1],
					adjacency[adjIndex + 2],
				};

				for (uint32_t i = 0; i < 3u; ++i) {
					// Invalid triangle in adjacency slot
					if (adj[i] == -1)
						continue;

					// Already processed triangle
					if (checklist[adj[i]])
						continue;

					// Triangle already in the candidate list
					if (candidateCheck.count(adj[i]))
						continue;

					candidates.push_back(std::make_pair(adj[i], FLT_MAX));
					candidateCheck.insert(adj[i]);
				}

				// Re-score remaining candidate triangles
				for (uint32_t i = 0; i < static_cast<uint32_t>(candidates.size()); ++i) {
					uint32_t candidate = candidates[i].first;

					uint32 triIndices[3] =
					{
						indices[candidate * 3],
						indices[candidate * 3 + 1],
						indices[candidate * 3 + 2],
					};

					assert(triIndices[0] < vertexCount);
					assert(triIndices[1] < vertexCount);
					assert(triIndices[2] < vertexCount);

					XMFLOAT3 triVerts[3] =
					{
						positions[triIndices[0]],
						positions[triIndices[1]],
						positions[triIndices[2]],
					};

					candidates[i].second = ComputeScore(*curr, psphere, normal, triIndices, triVerts);
				}

				// Determine whether we need to move to the next meshlet.
				if (IsMeshletFull(maxVerts, maxPrims, *curr)) {
					m_positions.clear();
					normals.clear();
					candidateCheck.clear();

					// Use one of our existing candidates as the next meshlet seed.
					if (!candidates.empty()) {
						candidates[0] = candidates.back();
						candidates.resize(1);
						candidateCheck.insert(candidates[0].first);
					}

					output.emplace_back();
					curr = &output.back();
				}
				else {
					std::sort(candidates.begin(), candidates.end(), &CompareScores);
				}
			}
			else {
				if (candidates.empty()) {
					m_positions.clear();
					normals.clear();
					candidateCheck.clear();

					output.emplace_back();
					curr = &output.back();
				}
			}

			// Ran out of candidates; add a new seed candidate to start the next meshlet.
			if (candidates.empty()) {
				while (triIndex < triCount && checklist[triIndex])
					++triIndex;

				if (triIndex == triCount)
					break;

				candidates.push_back(std::make_pair(triIndex, 0.0f));
				candidateCheck.insert(triIndex);
			}
		}

		// The last meshlet may have never had any primitives added to it - in which case we want to remove it.
		if (output.back().PrimitiveIndices.empty()) {
			output.pop_back();
		}
	}

	HRESULT ComputeMeshlets(
		uint32_t maxVerts, uint32_t maxPrims,
		const uint32* indices, uint32_t indexCount,
		const Subset* indexSubsets, uint32_t subsetCount,
		const DirectX::XMFLOAT3* positions, uint32_t vertexCount,
		std::vector<Subset>& meshletSubsets,
		std::vector<Meshlet>& meshlets,
		std::vector<uint32>& uniqueVertexIndices,
		std::vector<PackedTriangle>& primitiveIndices) {
		UNREFERENCED_PARAMETER(indexCount);

		for (uint32_t i = 0; i < subsetCount; ++i) {
			Subset s = indexSubsets[i];

			assert(s.Offset + s.Count <= indexCount);

			std::vector<InlineMeshlet> builtMeshlets;
			Meshletize(maxVerts, maxPrims, indices + s.Offset, s.Count, positions, vertexCount, builtMeshlets);

			Subset meshletSubset;
			meshletSubset.Offset = static_cast<uint32_t>(meshlets.size());
			meshletSubset.Count = static_cast<uint32_t>(builtMeshlets.size());
			meshletSubsets.push_back(meshletSubset);

			// Determine final unique vertex index and primitive index counts & offsets.
			uint32_t startVertCount = static_cast<uint32_t>(uniqueVertexIndices.size());
			uint32_t startPrimCount = static_cast<uint32_t>(primitiveIndices.size());

			uint32_t uniqueVertexIndexCount = startVertCount;
			uint32_t primitiveIndexCount = startPrimCount;

			// Resize the meshlet output array to hold the newly formed meshlets.
			uint32_t meshletCount = static_cast<uint32_t>(meshlets.size());
			meshlets.resize(meshletCount + builtMeshlets.size());

			for (uint32_t j = 0, dest = meshletCount; j < static_cast<uint32_t>(builtMeshlets.size()); ++j, ++dest) {
				meshlets[dest].VertOffset = uniqueVertexIndexCount;
				meshlets[dest].VertCount = static_cast<uint32_t>(builtMeshlets[j].UniqueVertexIndices.size());
				uniqueVertexIndexCount += static_cast<uint32_t>(builtMeshlets[j].UniqueVertexIndices.size());

				meshlets[dest].PrimOffset = primitiveIndexCount;
				meshlets[dest].PrimCount = static_cast<uint32_t>(builtMeshlets[j].PrimitiveIndices.size());
				primitiveIndexCount += static_cast<uint32_t>(builtMeshlets[j].PrimitiveIndices.size());
			}

			// Allocate space for the new data.
			uniqueVertexIndices.resize(uniqueVertexIndexCount);
			primitiveIndices.resize(primitiveIndexCount);

			// Copy data from the freshly built meshlets into the output buffers.
			auto vertDest = reinterpret_cast<uint32*>(uniqueVertexIndices.data()) + startVertCount;
			auto primDest = reinterpret_cast<uint32*>(primitiveIndices.data()) + startPrimCount;

			for (uint32_t j = 0; j < static_cast<uint32>(builtMeshlets.size()); ++j) {
				std::memcpy(vertDest, builtMeshlets[j].UniqueVertexIndices.data(), builtMeshlets[j].UniqueVertexIndices.size() * sizeof(uint32));
				std::memcpy(primDest, builtMeshlets[j].PrimitiveIndices.data(), builtMeshlets[j].PrimitiveIndices.size() * sizeof(uint32_t));

				vertDest += builtMeshlets[j].UniqueVertexIndices.size();
				primDest += builtMeshlets[j].PrimitiveIndices.size();
			}
		}

		return S_OK;
	}


	//
	// Strongly influenced by https://github.com/zeux/meshoptimizer - Thanks amigo!
	//
	HRESULT ComputeCullData(
		const XMFLOAT3* positions, uint32_t vertexCount,
		const Meshlet* meshlets, uint32_t meshletCount,
		const uint32* uniqueVertexIndices,
		const PackedTriangle* primitiveIndices,
		DWORD flags,
		CullData* cullData
	) {
		UNREFERENCED_PARAMETER(vertexCount);

		XMFLOAT3 vertices[256];
		XMFLOAT3 normals[256];

		for (uint32_t mi = 0; mi < meshletCount; ++mi) {
			auto& m = meshlets[mi];
			auto& c = cullData[mi];

			// Cache vertices
			for (uint32_t i = 0; i < m.VertCount; ++i) {
				uint32_t vIndex = uniqueVertexIndices[m.VertOffset + i];

				assert(vIndex < vertexCount);
				vertices[i] = positions[vIndex];
			}

			// Generate primitive normals & cache
			for (uint32_t i = 0; i < m.PrimCount; ++i) {
				auto primitive = primitiveIndices[m.PrimOffset + i];

				XMVECTOR triangle[3]
				{
					XMLoadFloat3(&vertices[primitive.indices.i0]),
					XMLoadFloat3(&vertices[primitive.indices.i1]),
					XMLoadFloat3(&vertices[primitive.indices.i2]),
				};

				XMVECTOR p10 = triangle[1] - triangle[0];
				XMVECTOR p20 = triangle[2] - triangle[0];
				XMVECTOR n = XMVector3Normalize(XMVector3Cross(p10, p20));

				XMStoreFloat3(&normals[i], (flags & CNORM_WIND_CW) != 0 ? -n : n);
			}

			// Calculate spatial bounds
			XMVECTOR positionBounds = MinimumBoundingSphere(vertices, m.VertCount);
			XMVECTOR boundsMin;
			XMVECTOR boundsMax;
			MinMaxBoundingBox(vertices, m.VertCount, boundsMin, boundsMax);
			XMStoreFloat3(&c.BoundingBoxMin, boundsMin);
			XMStoreFloat3(&c.BoundingBoxMax, boundsMax);

			// Calculate the normal cone
			// 1. Normalized center point of minimum bounding sphere of unit normals == conic axis
			XMVECTOR normalBounds = MinimumBoundingSphere(normals, m.PrimCount);

			// 2. Calculate dot product of all normals to conic axis, selecting minimum
			XMVECTOR axis = XMVectorSetW(XMVector3Normalize(normalBounds), 0);

			XMVECTOR minDot = g_XMOne;
			for (uint32_t i = 0; i < m.PrimCount; ++i) {
				XMVECTOR dot = XMVector3Dot(axis, XMLoadFloat3(&normals[i]));
				minDot = XMVectorMin(minDot, dot);
			}

			if (XMVector4Less(minDot, XMVectorReplicate(0.1f))) {
				// Degenerate cone
				c.NormalCone[0] = 127;
				c.NormalCone[1] = 127;
				c.NormalCone[2] = 127;
				c.NormalCone[3] = 255;
				continue;
			}

			// Find the point on center-t*axis ray that lies in negative half-space of all triangles
			float maxt = 0;

			for (uint32_t i = 0; i < m.PrimCount; ++i) {
				auto primitive = primitiveIndices[m.PrimOffset + i];

				uint32_t indices[3]
				{
					primitive.indices.i0,
					primitive.indices.i1,
					primitive.indices.i2,
				};

				XMVECTOR triangle[3]
				{
					XMLoadFloat3(&vertices[indices[0]]),
					XMLoadFloat3(&vertices[indices[1]]),
					XMLoadFloat3(&vertices[indices[2]]),
				};

				XMVECTOR c = positionBounds - triangle[0];

				XMVECTOR n = XMLoadFloat3(&normals[i]);
				float dc = XMVectorGetX(XMVector3Dot(c, n));
				float dn = XMVectorGetX(XMVector3Dot(axis, n));

				// dn should be larger than mindp cutoff above
				assert(dn > 0.0f);
				float t = dc / dn;

				maxt = (t > maxt) ? t : maxt;
			}

			// cone apex should be in the negative half-space of all cluster triangles by construction
			c.ApexOffset = maxt;

			// cos(a) for normal cone is minDot; we need to add 90 degrees on both sides and invert the cone
			// which gives us -cos(a+90) = -(-sin(a)) = sin(a) = sqrt(1 - cos^2(a))
			XMVECTOR coneCutoff = XMVectorSqrt(g_XMOne - minDot * minDot);

			// 3. Quantize to uint8
			XMVECTOR quantized = QuantizeSNorm(axis);
			c.NormalCone[0] = (uint8_t)XMVectorGetX(quantized);
			c.NormalCone[1] = (uint8_t)XMVectorGetY(quantized);
			c.NormalCone[2] = (uint8_t)XMVectorGetZ(quantized);

			XMVECTOR error = ((quantized / 255.0f) * 2.0f - g_XMOne) - axis;
			error = XMVectorSum(XMVectorAbs(error));

			quantized = QuantizeUNorm(coneCutoff + error);
			quantized = XMVectorMin(quantized + g_XMOne, XMVectorReplicate(255.0f));
			c.NormalCone[3] = (uint8_t)XMVectorGetX(quantized);
		}

		return S_OK;
	}
}


struct RawVertex {
	Float3 position;
	Float3 normal;
	Float3 tangent;
	Float2 texcoord;

	bool operator==(const RawVertex& left) const {
		return position == left.position && texcoord == left.texcoord && normal == left.normal && tangent == left.tangent;
	}
};

struct SubMeshInfo {
	uint32 materialSlotIndex;
	uint32 meshletCount;
	uint32 meshletOffset;
	uint32 triangleStripIndexCount;
	uint32 triangleStripIndexOffset;
};

struct LodInfo {
	uint32 vertexOffset = 0;
	uint32 vertexIndexOffset = 0;
	uint32 primitiveOffset = 0;
	uint32 indexOffset = 0;
	uint32 subMeshOffset = 0;
	uint32 subMeshCount = 0;
	uint32 vertexCount = 0;
	uint32 vertexIndexCount = 0;
	uint32 primitiveCount = 0;
};

struct MeshletPrimitiveInfo {
	uint32 _vertexIndexOffset;
	uint32 _vertexCount;
	uint32 _primitiveOffset;
	uint32 _primitiveCount;
};

struct MeshletCullInfo {
	Float3 _aabbMin;
	Float3 _aabbMax;
	uint32 _normalAndCutoff;
	float _apexOffset;
};

struct VertexPolygonRef {
	VectorArray<uint32> _polygonIndices;
};

struct PolygonInfo {
	uint32 _polygonIndices[3];
	uint32 _globalMaterialIndex;
	uint32 _localMaterialIndex;
	Vector3 _centerPosition;

	// ポリゴンの法線を取得
	Vector3 getFaceNormal(const VectorArray<Float3>& positions) const {
		Vector3 v1 = (Vector3(positions[_polygonIndices[1]]) - Vector3(positions[_polygonIndices[0]]));
		Vector3 v2 = (Vector3(positions[_polygonIndices[2]]) - Vector3(positions[_polygonIndices[1]]));
		return Vector3::Cross(v1, v2).getNormalize();
	}

	// ポリゴンの中心座標を取得
	Vector3 getCentorPosition(const VectorArray<Float3>& positions) const {
		return (Vector3(positions[_polygonIndices[0]]) + Vector3(positions[_polygonIndices[1]]) + Vector3(positions[_polygonIndices[2]])) / 3.0f;
	}

	// 引数のポリゴンが隣接しているか
	bool isAdjacent(const PolygonInfo& info) const {
		for (uint32 i = 0; i < 3; ++i) {
			uint32 index = _polygonIndices[i];
			for (uint32 j = 0; j < 3; ++j) {
				if (index == info._polygonIndices[j]) {
					return true;
				}
			}
		}
		return false;
	}

	// ポリゴンを包含するバウンディングスフィアの半径を取得
	float getRadius(const VectorArray<Float3>& positions) const {
		Vector3 centerPosition = getCentorPosition(positions);
		float bestRadiusSqr = FLT_MAX;
		uint32 bestIndex = 0;
		for (uint32 ply = 0; ply < 3; ++ply) {
			float lengthSqr = (centerPosition - Vector3(positions[_polygonIndices[0]])).getLengthSqr();
			if (lengthSqr < bestRadiusSqr) {
				bestRadiusSqr = lengthSqr;
				bestIndex = ply;
			}
		}

		return Sqr(bestRadiusSqr);
	}
};

#define HashCombine(hash,seed) hash + 0x9e3779b9 + (seed << 6) + (seed >> 2)

namespace std {
	template<>
	class hash<RawVertex> {
	public:

		size_t operator () (const RawVertex& p) const {
			size_t seed = 0;

			seed ^= HashCombine(hash<float>()(p.position.x), seed);
			seed ^= HashCombine(hash<float>()(p.position.y), seed);
			seed ^= HashCombine(hash<float>()(p.position.z), seed);
			seed ^= HashCombine(hash<float>()(p.normal.x), seed);
			seed ^= HashCombine(hash<float>()(p.normal.y), seed);
			seed ^= HashCombine(hash<float>()(p.normal.z), seed);
			seed ^= HashCombine(hash<float>()(p.texcoord.x), seed);
			seed ^= HashCombine(hash<float>()(p.texcoord.y), seed);
			return seed;
		}
	};
}

uint32 packTexCoords(Float2 coords) {
	uint32 result = 0;
	uint16* packed = reinterpret_cast<uint16*>(&result);
	packed[0] = ((coords.x + 4.0f) / 8.0f) * UINT16_MAX; // -4.0 ~ 4.0
	packed[1] = ((coords.y + 4.0f) / 8.0f) * UINT16_MAX; // -4.0 ~ 4.0
	return result;
}

bool isMesh(FbxNode* node) {
	if (node) {
		int attrCount = node->GetNodeAttributeCount();

		for (int i = 0; attrCount > i; i++) {
			FbxNodeAttribute::EType attrType = node->GetNodeAttributeByIndex(i)->GetAttributeType();

			// ノードがメッシュにつながっているかチェック
			if (attrType == FbxNodeAttribute::EType::eMesh) {
				return true;
			}
		}
	}
	return false;
}

void probeNode(FbxNode* node, std::vector<FbxMesh*>& meshes) {
	if (node) {
		int attrCount = node->GetNodeAttributeCount();

		for (int i = 0; i < attrCount; i++) {
			FbxNodeAttribute::EType attrType = node->GetNodeAttributeByIndex(i)->GetAttributeType();

			// ノードがメッシュにつながっているかチェック
			if (attrType == FbxNodeAttribute::EType::eMesh) {
				meshes.push_back(node->GetMesh());
			}
		}

		for (int i = 0; node->GetChildCount() > i; i++) {
			probeNode(node->GetChild(i), meshes);
		}
	}
}

void exportMesh(const char* fileName) {
	std::vector<FbxMesh*> _fbxMeshes;
	std::vector<Subset>                     m_meshletSubsets;
	std::vector<Meshlet>                    m_meshlets;
	std::vector<uint32>                     m_uniqueVertexIndices;
	std::vector<PackedTriangle>             m_primitiveIndices;
	std::vector<CullData>                   m_cullData;
	std::vector<Subset>                     m_subSets;
	std::unordered_map<size_t, uint32_t>    m_uniqueVertices;
	std::vector<uint32_t>                   m_faceRemap;
	std::vector<uint32_t>                   m_vertexRemap;
	std::vector<uint8_t>                    m_indexReorder;
	std::vector<uint32_t>                   m_dupVerts;
	std::vector<uint32>                     m_classicIndices;

	fbxsdk::FbxManager* manager = fbxsdk::FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);
	FbxScene* scene = FbxScene::Create(manager, "");

	FbxImporter* importer = FbxImporter::Create(manager, "");
	String fullPath = fileName;
	bool isSuccsess = importer->Initialize(fullPath.c_str(), -1, manager->GetIOSettings());
	if (!isSuccsess) {
		std::cout << LOG_COLOR_RED << "Faild open file " << fullPath << LOG_COLOR_END << std::endl;
		return;
	}

	importer->Import(scene);
	importer->Destroy();

	FbxGeometryConverter geometryConverter(manager);
	geometryConverter.Triangulate(scene, true);

	FbxAxisSystem::DirectX.ConvertScene(scene);

	// シーン内からFBX Meshノードを検索抽出
	probeNode(scene->GetRootNode(), _fbxMeshes);

	constexpr float GLOBAL_SCALE = 0.01f;// meter to cm
	constexpr uint32 MESHLET_INDEX_COUNT = 126;
	constexpr uint32 MESHLET_POLYGON_COUNT = MESHLET_INDEX_COUNT / 3;

	constexpr uint32 MESHLT_VERTEX_COUNT_MAX = 64;
	constexpr uint32 MESHLET_PRIMITIVE_COUNT_MAX = 126;
	constexpr float FLOAT_MAX = FLT_MAX;
	constexpr float FLOAT_MIN = -FLT_MAX;

	uint32 lodCount = static_cast<uint32>(_fbxMeshes.size());
	VectorArray<LodInfo> lodInfos(lodCount);
	VectorArray<SubMeshInfo> subMeshInfos;
	VectorArray<ullong64> materialNameHashes;
	VectorArray<uint32> materialSlotIndices;
	VectorArray<Float3> positionsL;
	VectorArray<uint32> normalsAndTangentsL;
	VectorArray<uint32> texcoordsL;
	VectorArray<uint32> triangleStripIndexOffsets;
	VectorArray<uint32> triangleStripIndexCounts;
	for (uint32 lodIndex = 0; lodIndex < lodCount; ++lodIndex) {
		FbxMesh* mesh = _fbxMeshes[lodIndex];
		const uint32 materialCount = mesh->GetNode()->GetMaterialCount();
		const uint32 vertexCount = mesh->GetControlPointsCount();
		const uint32 polygonCount = mesh->GetPolygonCount();
		const uint32 polygonVertexCount = 3;
		const uint32 indexCount = polygonCount * polygonVertexCount;
		const uint32 meshletCountMax = indexCount / MESHLET_INDEX_COUNT + 1;

		FbxStringList uvSetNames;
		bool bIsUnmapped = false;
		mesh->GetUVSetNames(uvSetNames);

		VectorArray<uint32> globalRemapedMaterialIndices(materialCount);
		VectorArray<uint32> localRemapedMaterialIndices(materialCount);
		VectorArray<ullong64> localMaterialNameHashes;

		// マテリアル名から 64bit hash を収集する
		for (uint32 i = 0; i < materialCount; ++i) {
			const char* name = mesh->GetNode()->GetMaterial(i)->GetName();
			const char* namePtr = name;
			uint32 nameLength = 0;
			while (*namePtr != '\0') {
				nameLength++;
				namePtr++;
			}

			uint32 findMaterialIndex = static_cast<uint32>(-1);
			ullong64 nameHash = XXH64(name, nameLength, 0);
			for (uint32 i = 0; i < localMaterialNameHashes.size(); ++i) {
				if (localMaterialNameHashes[i] == nameHash) {
					findMaterialIndex = i;
					break;
				}
			}

			if (findMaterialIndex == static_cast<uint32>(-1)) {
				localRemapedMaterialIndices[i] = static_cast<uint32>(localMaterialNameHashes.size());
				localMaterialNameHashes.push_back(nameHash);
				continue;
			}
			localRemapedMaterialIndices[i] = findMaterialIndex;
		}

		// マテリアル名ハッシュをグローバルに登録
		for (size_t i = 0; i < materialCount; ++i) {
			uint32 remapedIndex = localRemapedMaterialIndices[i];
			uint32 foundIndex = static_cast<uint32>(-1);
			for (size_t j = 0; j < materialNameHashes.size(); ++j) {
				if (materialNameHashes[j] == localMaterialNameHashes[remapedIndex]) {
					foundIndex = static_cast<uint32>(j);
					break;
				}
			}

			if (foundIndex == static_cast<uint32>(-1)) {
				globalRemapedMaterialIndices[i] = static_cast<uint32>(materialNameHashes.size());
				materialNameHashes.push_back(localMaterialNameHashes[remapedIndex]);
				continue;
			}

			globalRemapedMaterialIndices[i] = foundIndex;
		}

		//マテリアルごとの頂点インデックス数を調べる
		FbxLayerElementMaterial* meshMaterials = mesh->GetLayer(0)->GetMaterials();
		FbxLayerElement::EMappingMode materialMappingMode = meshMaterials ?
			meshMaterials->GetMappingMode() : FbxLayerElement::eByPolygon;

		VectorArray<uint32> materialIndexSizes(localMaterialNameHashes.size());
		for (uint32 i = 0; i < polygonCount; ++i) {
			uint32 rowMaterialIndex = 0;
			switch (materialMappingMode) {
			case FbxLayerElement::eAllSame:
				rowMaterialIndex = meshMaterials->GetIndexArray().GetAt(0);
				break;
			case FbxLayerElement::eByPolygon:
				rowMaterialIndex = meshMaterials->GetIndexArray().GetAt(i);
				break;
			}
			uint32 materialIndex = localRemapedMaterialIndices[rowMaterialIndex];
			materialIndexSizes[materialIndex] += polygonVertexCount;
		}

		//マテリアルごとのインデックスオフセットを計算
		VectorArray<uint32> materialIndexOffsets(localMaterialNameHashes.size());
		for (size_t i = 0; i < materialIndexOffsets.size(); ++i) {
			for (size_t j = 0; j < i; ++j) {
				materialIndexOffsets[i] += materialIndexSizes[j];
			}
		}

		UnorderedMap<RawVertex, uint32> optimizedVertices;// 重複しない頂点情報と新しい頂点インデックス
		VectorArray<VertexPolygonRef> vertexRefInfos(indexCount);// 参照しているポリゴンインデックス
		VectorArray<PolygonInfo> polygonInfos(polygonCount);// ポリゴンの各頂点インデックス
		VectorArray<uint32> polygonInfoCounters(localMaterialNameHashes.size());
		VectorArray<uint32> indices(indexCount);//新しい頂点インデックスでできたインデックスバッファ
		VectorArray<uint32> materialIndexCounter(localMaterialNameHashes.size());//マテリアルごとのインデックス数を管理
		for (auto&& vertexRefInfo : vertexRefInfos) {
			vertexRefInfo._polygonIndices.reserve(32);
		}

		optimizedVertices.reserve(indexCount);

		for (uint32 sourcePolygonIndex = 0; sourcePolygonIndex < polygonCount; ++sourcePolygonIndex) {
			uint32 rowMaterialIndex = meshMaterials->GetIndexArray().GetAt(sourcePolygonIndex);
			uint32 materialIndex = localRemapedMaterialIndices[rowMaterialIndex];
			uint32 globalMaterialIndex = globalRemapedMaterialIndices[rowMaterialIndex];
			uint32 materialIndexOffset = materialIndexOffsets[materialIndex];

			uint32 materialPolygonOffset = materialIndexOffset / 3;
			uint32 materialPolygonCounter = polygonInfoCounters[materialIndex]++;
			uint32 materialPolygonIndex = materialPolygonOffset + materialPolygonCounter;

			uint32& indexCount = materialIndexCounter[materialIndex];
			PolygonInfo& polygonInfo = polygonInfos[materialPolygonIndex];
			polygonInfo._globalMaterialIndex = globalMaterialIndex;
			polygonInfo._localMaterialIndex = materialIndex;

			for (uint32 j = 0; j < polygonVertexCount; ++j) {
				const uint32 rowVertexIndex = mesh->GetPolygonVertex(sourcePolygonIndex, j);
				FbxVector4 v = mesh->GetControlPointAt(rowVertexIndex) * GLOBAL_SCALE;
				FbxVector4 normal;
				FbxVector2 texcoord;

				FbxString uvSetName = uvSetNames.GetStringAt(0);//UVSetは０番インデックスのみ対応
				mesh->GetPolygonVertexUV(sourcePolygonIndex, j, uvSetName, texcoord, bIsUnmapped);
				mesh->GetPolygonVertexNormal(sourcePolygonIndex, j, normal);

				RawVertex r;
				r.position = { (float)v[0], (float)v[1], -(float)v[2] };//FBXは右手座標系なので左手座標系に直すためにZを反転する
				r.normal = { (float)normal[0], (float)normal[1], -(float)normal[2] };
				r.texcoord = { (float)texcoord[0], 1 - (float)texcoord[1] };//UVのY軸を反転

				const XMVECTOR vectorUp = XMVectorSet(0.0f, 1, FLT_EPSILON, 0);
				XMVECTOR tangent = XMVector3Cross(XMLoadFloat3(&r.normal), vectorUp);
				XMStoreFloat3(&r.tangent, tangent);

				//Zを反転するとポリゴンが左回りになるので右回りになるようにインデックスを0,1,2 → 2,1,0にする
				uint32 invLocalPolygonIndex = 2 - j;
				uint32 indexInverseCorrectionedValue = indexCount + invLocalPolygonIndex;
				uint32 indexPerMaterial = materialIndexOffset + indexInverseCorrectionedValue;
				uint32 vertexIndex = 0;
				if (optimizedVertices.count(r) == 0) {
					vertexIndex = static_cast<uint32>(optimizedVertices.size());
					optimizedVertices.emplace(r, vertexIndex);
				}
				else {
					vertexIndex = optimizedVertices.at(r);
				}

				// ポリゴンインデックスを記録
				indices[indexPerMaterial] = vertexIndex;
				polygonInfo._polygonIndices[invLocalPolygonIndex] = vertexIndex;

				// 頂点の参照しているポリゴンリストに追加
				vertexRefInfos[vertexIndex]._polygonIndices.push_back(materialPolygonIndex);
			}

			indexCount += polygonVertexCount;
		}


		//UnorederedMapの配列からVectorArrayに変換
		VectorArray<RawVertex> vertices(optimizedVertices.size());
		VectorArray<Float3> positions(optimizedVertices.size());
		VectorArray<uint32> normalAndTangents(optimizedVertices.size());
		VectorArray<uint32> texcoords(optimizedVertices.size());
		for (const auto& vertex : optimizedVertices) {
			vertices[vertex.second] = vertex.first;
		}

		// zeux/meshoptimizer https://github.com/zeux/meshoptimizer/blob/master/src/meshoptimizer.h
		meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());
		meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(), vertices.data(), vertices.size(), sizeof(RawVertex));

		for (uint32 v = 0; v < vertices.size(); ++v) {
			const RawVertex& vertex = vertices[v];
			positions[v] = vertex.position;
			normalAndTangents[v] = packNormalAndTangentOctahedron(vertex.normal, vertex.tangent);
			texcoords[v] = packTexCoords(vertex.texcoord);
		}

		uint32 totalIndexCount = 0;
		uint32 subMeshCount = localMaterialNameHashes.size();
		std::vector<uint32> subMeshMaterialSlotIndices(subMeshCount);
		std::vector<Subset> subSets(subMeshCount);
		for (uint32 subSetIndex = 0; subSetIndex < subMeshCount; ++subSetIndex) {
			subMeshMaterialSlotIndices[subSetIndex] = globalRemapedMaterialIndices[subSetIndex];

			Subset& subSet = subSets[subSetIndex];
			subSet.Count = materialIndexSizes[subSetIndex];
			subSet.Offset = materialIndexOffsets[subSetIndex];
			totalIndexCount += subSet.Count;
		}

		m_subSets.reserve(m_subSets.size() + subSets.size());
		std::copy(subSets.begin(), subSets.end(), std::back_inserter(m_subSets));

		materialSlotIndices.reserve(materialSlotIndices.size() + subMeshMaterialSlotIndices.size());
		std::copy(subMeshMaterialSlotIndices.begin(), subMeshMaterialSlotIndices.end(), std::back_inserter(materialSlotIndices));

		std::vector<Subset>                     meshletSubsets;
		std::vector<Meshlet>                    meshlets;
		std::vector<uint32>                     uniqueVertexIndices;
		std::vector<PackedTriangle>             primitiveIndices;
		std::vector<CullData>                   cullData;

		// Meshletize our mesh and generate per-meshlet culling data
		ComputeMeshlets(
			MESHLT_VERTEX_COUNT_MAX, MESHLET_PRIMITIVE_COUNT_MAX,
			indices.data(), indexCount,
			subSets.data(), static_cast<uint32>(subSets.size()),
			positions.data(), static_cast<uint32>(positions.size()),
			meshletSubsets,
			meshlets,
			uniqueVertexIndices,
			primitiveIndices
		);

		cullData.resize(meshlets.size());
		ComputeCullData(
			positions.data(), static_cast<uint32_t>(positions.size()),
			meshlets.data(), static_cast<uint32_t>(meshlets.size()),
			uniqueVertexIndices.data(),
			primitiveIndices.data(),
			CNORM_DEFAULT,
			cullData.data()
		);

		LodInfo& lodInfo = lodInfos[lodIndex];
		lodInfo.subMeshCount = subMeshCount;
		lodInfo.vertexCount = positions.size();
		lodInfo.vertexIndexCount = uniqueVertexIndices.size();
		lodInfo.primitiveCount = primitiveIndices.size();

		m_meshletSubsets.reserve(m_meshletSubsets.size() + meshletSubsets.size());
		std::copy(meshletSubsets.begin(), meshletSubsets.end(), std::back_inserter(m_meshletSubsets));

		m_meshlets.reserve(m_meshlets.size() + meshlets.size());
		std::copy(meshlets.begin(), meshlets.end(), std::back_inserter(m_meshlets));

		m_uniqueVertexIndices.reserve(m_uniqueVertexIndices.size() + uniqueVertexIndices.size());
		std::copy(uniqueVertexIndices.begin(), uniqueVertexIndices.end(), std::back_inserter(m_uniqueVertexIndices));

		m_primitiveIndices.reserve(m_primitiveIndices.size() + primitiveIndices.size());
		std::copy(primitiveIndices.begin(), primitiveIndices.end(), std::back_inserter(m_primitiveIndices));

		m_cullData.reserve(m_cullData.size() + cullData.size());
		std::copy(cullData.begin(), cullData.end(), std::back_inserter(m_cullData));

		positionsL.reserve(positionsL.size() + positions.size());
		std::copy(positions.begin(), positions.end(), std::back_inserter(positionsL));

		texcoordsL.reserve(texcoordsL.size() + texcoords.size());
		std::copy(texcoords.begin(), texcoords.end(), std::back_inserter(texcoordsL));

		normalsAndTangentsL.reserve(normalsAndTangentsL.size() + normalAndTangents.size());
		std::copy(normalAndTangents.begin(), normalAndTangents.end(), std::back_inserter(normalsAndTangentsL));

		m_classicIndices.reserve(m_classicIndices.size() + indices.size());
		std::copy(indices.begin(), indices.end(), std::back_inserter(m_classicIndices));

		std::cout << "Lod Mesh :" << lodIndex << std::endl;
		std::cout << "Sub Mesh :" << subMeshCount << std::endl;

		std::cout << "Compute Cul Lod Mesh :" << lodIndex << std::endl;

	}

	manager->Destroy();

	//BoundingBoxのサイズを計算
	Float3 aabbMin = { FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
	Float3 aabbMax = { FLOAT_MIN, FLOAT_MIN, FLOAT_MIN };
	for (const auto& v : positionsL) {
		if (v.x < aabbMin.x) { aabbMin.x = v.x; }
		if (v.x > aabbMax.x) { aabbMax.x = v.x; }
		if (v.y < aabbMin.y) { aabbMin.y = v.y; }
		if (v.y > aabbMax.y) { aabbMax.y = v.y; }
		if (v.z < aabbMin.z) { aabbMin.z = v.z; }
		if (v.z > aabbMax.z) { aabbMax.z = v.z; }
	}

	for (uint32 lodIndex = 1; lodIndex < lodCount; ++lodIndex) {
		LodInfo& lodInfo = lodInfos[lodIndex];
		const LodInfo& prevLodInfo = lodInfos[lodIndex - 1];
		lodInfo.subMeshOffset = prevLodInfo.subMeshOffset + prevLodInfo.subMeshCount;
		lodInfo.vertexOffset = prevLodInfo.vertexOffset + prevLodInfo.vertexCount;
		lodInfo.vertexIndexOffset = prevLodInfo.vertexIndexOffset + prevLodInfo.vertexIndexCount;
		lodInfo.primitiveOffset = prevLodInfo.primitiveOffset + prevLodInfo.primitiveCount;
	}

	uint32 subMeshCount = m_meshletSubsets.size();
	subMeshInfos.resize(subMeshCount);
	for (uint32 subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex) {
		SubMeshInfo& info = subMeshInfos[subMeshIndex];
		info.materialSlotIndex = materialSlotIndices[subMeshIndex];
		info.meshletCount = m_meshletSubsets[subMeshIndex].Count;
		info.triangleStripIndexCount = m_subSets[subMeshIndex].Count;
	}

	for (uint32 subMeshIndex = 1; subMeshIndex < subMeshCount; ++subMeshIndex) {
		SubMeshInfo& info = subMeshInfos[subMeshIndex];
		const SubMeshInfo& prevInfo = subMeshInfos[subMeshIndex - 1];
		info.meshletOffset = prevInfo.meshletOffset + prevInfo.meshletCount;
		info.triangleStripIndexOffset = prevInfo.triangleStripIndexOffset + prevInfo.triangleStripIndexCount;
	}

	uint32 meshletCount = m_meshlets.size();
	VectorArray<MeshletCullInfo> meshletsL(meshletCount);
	for (uint32 meshletIndex = 0; meshletIndex < meshletCount; ++meshletIndex) {
		const CullData& cullData = m_cullData[meshletIndex];
		MeshletCullInfo& info = meshletsL[meshletIndex];
		info._aabbMax = Float3(cullData.BoundingBoxMax.x, cullData.BoundingBoxMax.y, cullData.BoundingBoxMax.z);
		info._aabbMin = Float3(cullData.BoundingBoxMin.x, cullData.BoundingBoxMin.y, cullData.BoundingBoxMin.z);
		info._apexOffset = cullData.ApexOffset;
		memcpy(&info._normalAndCutoff, cullData.NormalCone, sizeof(uint32));
	}

	VectorArray<MeshletPrimitiveInfo> meshletPrimitiveL(meshletCount);
	for (uint32 meshletIndex = 0; meshletIndex < meshletCount; ++meshletIndex) {
		const Meshlet& meshlet = m_meshlets[meshletIndex];
		MeshletPrimitiveInfo& info = meshletPrimitiveL[meshletIndex];
		info._vertexIndexOffset = meshlet.VertOffset;
		info._vertexCount = meshlet.VertCount;
		info._primitiveCount = meshlet.PrimCount;
		info._primitiveOffset = meshlet.PrimOffset;
	}

	uint32 meshletVertexIndexCount = static_cast<uint32>(m_uniqueVertexIndices.size());
	uint32 meshletPrimitiveCount = static_cast<uint32>(m_primitiveIndices.size());
	uint32 materialCount = static_cast<uint32>(materialNameHashes.size());
	uint32 verticesCount = static_cast<uint32>(positionsL.size());
	uint32 classicIndexCount = static_cast<uint32>(m_classicIndices.size());
	uint32 positionVerticesSize = verticesCount * sizeof(Float3);
	uint32 normalAndTangentVerticesSize = verticesCount * sizeof(uint32);
	uint32 texcoordVerticesSize = verticesCount * sizeof(uint32);
	uint32 indicesSize = meshletVertexIndexCount * sizeof(uint32);
	uint32 primitiveSize = meshletPrimitiveCount * sizeof(uint32);
	uint32 subMeshSize = subMeshCount * sizeof(SubMeshInfo);
	uint32 boundingBoxSize = sizeof(XMFLOAT3) * 2;
	uint32 lodInfoSize = sizeof(LodInfo) * lodCount;
	uint32 meshletPrimitiveSize = sizeof(MeshletPrimitiveInfo) * meshletCount;
	uint32 meshletCullSize = sizeof(MeshletCullInfo) * meshletCount;
	uint32 materialNameHashesSize = sizeof(ullong64) * materialCount;
	uint32 classicIndexSize = sizeof(uint32) * classicIndexCount;

	std::string filePath(fileName);
	constexpr char WORK_FOLDER_PATH[] = "L:\\LightnEngine\\work";
	constexpr char RESOURCE_FOLDER_PATH[] = "L:\\LightnEngine\\resource";
	constexpr uint32 WORK_FILDER_PATH_LENGTH = _countof(WORK_FOLDER_PATH);
	constexpr uint32 EXT_STRING_SIZE = 4; // .fbx
	size_t fileNameSplitPoint = filePath.find_last_of("\\");
	std::string outputLocalPath = fullPath.substr(WORK_FILDER_PATH_LENGTH, fullPath.size() - WORK_FILDER_PATH_LENGTH - EXT_STRING_SIZE);

	{
		size_t folderSplitPoint = outputLocalPath.find_last_of("\\");
		std::string outputLocalFolder = outputLocalPath.substr(0, folderSplitPoint);

		char exportFolderPath[256] = {};
		sprintf_s(exportFolderPath, "%s\\%s", RESOURCE_FOLDER_PATH, outputLocalFolder.c_str());

		if (_mkdir(exportFolderPath) == 0) {
			char mkdirCommand[256] = {};
			sprintf_s(exportFolderPath, "%s %s", "mkdir", exportFolderPath);
			system(exportFolderPath);
		}
	}

	char exportPath[128] = {};
	sprintf_s(exportPath, "%s\\%s", RESOURCE_FOLDER_PATH, outputLocalPath.c_str());
	std::string exportFilePath = std::string(exportPath);
	exportFilePath.append(".mesh");

	std::ofstream fout;
	fout.open(exportFilePath, std::ios::out | std::ios::binary | std::ios::trunc);

	fout.write(reinterpret_cast<const char*>(&materialCount), 4);
	fout.write(reinterpret_cast<const char*>(&subMeshCount), 4);
	fout.write(reinterpret_cast<const char*>(&lodCount), 4);
	fout.write(reinterpret_cast<const char*>(&meshletCount), 4);
	fout.write(reinterpret_cast<const char*>(&verticesCount), 4);
	fout.write(reinterpret_cast<const char*>(&meshletVertexIndexCount), 4);
	fout.write(reinterpret_cast<const char*>(&meshletPrimitiveCount), 4);
	fout.write(reinterpret_cast<const char*>(&aabbMin), 12);
	fout.write(reinterpret_cast<const char*>(&aabbMax), 12);
	fout.write(reinterpret_cast<const char*>(&classicIndexCount), 4);
	fout.write(reinterpret_cast<const char*>(materialNameHashes.data()), materialNameHashesSize);
	fout.write(reinterpret_cast<const char*>(subMeshInfos.data()), subMeshSize);
	fout.write(reinterpret_cast<const char*>(lodInfos.data()), lodInfoSize);
	fout.write(reinterpret_cast<const char*>(meshletPrimitiveL.data()), meshletPrimitiveSize);
	fout.write(reinterpret_cast<const char*>(meshletsL.data()), meshletCullSize);
	fout.write(reinterpret_cast<const char*>(m_primitiveIndices.data()), primitiveSize);
	fout.write(reinterpret_cast<const char*>(m_uniqueVertexIndices.data()), indicesSize);
	fout.write(reinterpret_cast<const char*>(positionsL.data()), positionVerticesSize);
	fout.write(reinterpret_cast<const char*>(normalsAndTangentsL.data()), normalAndTangentVerticesSize);
	fout.write(reinterpret_cast<const char*>(texcoordsL.data()), texcoordVerticesSize);
	fout.write(reinterpret_cast<const char*>(m_classicIndices.data()), classicIndexSize);

	fout.close();
	std::cout << "Vertex Count: " << verticesCount << std::endl;
	std::cout << "Vertex Index Count: " << meshletVertexIndexCount << std::endl;
	std::cout << LOG_COLOR_GREEN << "Sucsessed convert " << exportFilePath << LOG_COLOR_END << std::endl;
}

int main(int argc, char* argv[]) {
	uint32 fileCount = argc - 1;
	VectorArray<String> fileNames(fileCount);

	std::cout << "Convert Target File " << fileCount << std::endl;
	for (uint32 i = 0; i < fileCount; ++i) {
		fileNames[i] = argv[i + 1];
		std::cout << "Name: " << fileNames[i] << std::endl;
	}
	std::cout << LOG_LINE << std::endl;

	for (const auto& fileName : fileNames) {
		exportMesh(fileName.c_str());
	}

	return 0;
}