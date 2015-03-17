#include "../core/algorithms.hpp"
#include "discretization.hpp"

namespace panoramix {
    namespace vis {
 
        using namespace core;

        namespace {

            inline Vec3 ToVec3Affine(const Vec4 & v4) {
                return Vec3(v4[0], v4[1], v4[2]) / v4[3];
            }

            inline Vec2 ToVec2(const Vec3 & v3) {
                return Vec2(v3[0], v3[1]);
            }

        }



        // tri mesh implementation
        TriMesh::Vertex::Vertex()
            : position(0, 0, 0, 1), normal(0, 0, 0), color(0, 0, 0, 1), texCoord(0, 0), entityIndex(-1), isSelected(false) {
        }


        TriMesh::VertHandle TriMesh::addVertex(const TriMesh::Vertex & v) {
            vertices.push_back(v);
            iPoints.push_back(static_cast<TriMesh::VertHandle>(vertices.size() - 1));
            return iPoints.back();
        }

        TriMesh::LineHandle TriMesh::addLine(TriMesh::VertHandle v1, TriMesh::VertHandle v2) {
            iLines.push_back(v1);
            iLines.push_back(v2);
            return iLines.size() / 2;
        }

        TriMesh::LineHandle TriMesh::addIsolatedLine(const Vertex & v1, const Vertex & v2) {
            vertices.push_back(v1);
            iLines.push_back(vertices.size() - 1);
            vertices.push_back(v2);
            iLines.push_back(vertices.size() - 1);
            return iLines.size() / 2;
        }

        size_t TriMesh::numberOfLines() const {
            return iLines.size() / 2;
        }

        void TriMesh::fetchLineVerts(TriMesh::LineHandle l, TriMesh::VertHandle & v1, TriMesh::VertHandle & v2) const {
            v1 = iLines[l * 2];
            v2 = iLines[l * 2 + 1];
        }

        TriMesh::TriangleHandle TriMesh::addTriangle(TriMesh::VertHandle v1, TriMesh::VertHandle v2, TriMesh::VertHandle v3) {
            iTriangles.push_back(v1);
            iTriangles.push_back(v2);
            iTriangles.push_back(v3);
            return iTriangles.size() / 3;
        }

        TriMesh::TriangleHandle TriMesh::addIsolatedTriangle(const Vertex & v1, const Vertex & v2, const Vertex & v3) {
            vertices.push_back(v1);
            iTriangles.push_back(vertices.size() - 1);
            vertices.push_back(v2);
            iTriangles.push_back(vertices.size() - 1);
            vertices.push_back(v3);
            iTriangles.push_back(vertices.size() - 1);
            return iTriangles.size() / 3;
        }

        size_t TriMesh::numberOfTriangles() const {
            return iTriangles.size() / 3;
        }

        void TriMesh::fetchTriangleVerts(TriMesh::TriangleHandle t, TriMesh::VertHandle & v1, TriMesh::VertHandle & v2, TriMesh::VertHandle & v3) const {
            v1 = iTriangles[t * 3];
            v2 = iTriangles[t * 3 + 1];
            v3 = iTriangles[t * 3 + 2];
        }

        void TriMesh::addQuad(TriMesh::VertHandle v1, TriMesh::VertHandle v2, TriMesh::VertHandle v3, TriMesh::VertHandle v4) {
            addTriangle(v1, v2, v3);
            addTriangle(v1, v3, v4);
        }

        void TriMesh::addPolygon(const std::vector<TriMesh::VertHandle> & vhs) {
            assert(vhs.size() >= 3);
            // get normal direction
            Vec3 normal = normalize(
                (ToVec3Affine(vertices[vhs[1]].position) - ToVec3Affine(vertices[vhs[0]].position)).cross(
                (ToVec3Affine(vertices[vhs[2]].position) - ToVec3Affine(vertices[vhs[1]].position)))
                );

            TriangulatePolygon(vhs.begin(), vhs.end(), [this, &normal](VertHandle vh) {
                Vec3 v = ToVec3Affine(vertices[vh].position);
                return ToVec2(v - v.dot(normal) * normal);
            }, [this](VertHandle a, VertHandle b, VertHandle c){
                addTriangle(a, b, c);
            });
        }

        void TriMesh::clear() {
            vertices.clear();
            iPoints.clear();
            iLines.clear();
            iTriangles.clear();
        }

        Box3 TriMesh::boundingBox() const {
            if (vertices.empty())
                return Box3();
            Box3 box(ToVec3Affine(vertices.front().position), ToVec3Affine(vertices.front().position));
            for (auto & v : vertices) {
                auto p = ToVec3Affine(v.position);
                box = box | BoundingBox(p);
            }
            return box;
        }












        void Discretize(TriMesh & mesh, const core::Sphere3 & s, const DiscretizeOptions & o) {
            int m = o.subdivisionNums[0];
            int n = o.subdivisionNums[1];
            if (!o.isolatedTriangles){
                mesh.vertices.reserve(mesh.vertices.size() + m * n);
                std::vector<std::vector<TriMesh::VertHandle>> vhs(m, std::vector<TriMesh::VertHandle>(n));
                for (int i = 0; i < m; i++) {
                    for (int j = 0; j < n; j++) {
                        float xratio = 1.0f / n * j;
                        float yratio = 1.0f / (m - 1) * i;
                        float xangle = M_PI * 2 * xratio;
                        float yangle = M_PI * yratio - M_PI_2;
                        Vec4 point = {
                            cos(xangle)*cos(yangle) * s.radius + s.center[0],
                            sin(xangle)*cos(yangle) * s.radius + s.center[1],
                            sin(yangle) * s.radius + s.center[2],
                            1
                        };
                        TriMesh::Vertex v;
                        v.position = point;
                        v.texCoord = { xratio, yratio };
                        v.color = o.color;
                        v.entityIndex = o.index;
                        vhs[i][j] = mesh.addVertex(v);
                    }
                }
                for (int i = 1; i < m; i++) {
                    int previ = i == 0 ? m - 1 : i - 1;
                    for (int j = 0; j < n; j++) {
                        int prevj = j == 0 ? n - 1 : j - 1;
                        mesh.addTriangle(vhs[i][j], vhs[i][prevj], vhs[previ][prevj]);
                        mesh.addTriangle(vhs[i][j], vhs[previ][prevj], vhs[previ][j]);
                    }
                }
            }
            else {
                std::vector<std::vector<TriMesh::Vertex>> vs(m, std::vector<TriMesh::Vertex>(n));
                for (int i = 0; i < m; i++) {
                    for (int j = 0; j < n; j++) {
                        float xratio = 1.0f / n * j;
                        float yratio = 1.0f / (m - 1) * i;
                        float xangle = M_PI * 2 * xratio;
                        float yangle = M_PI * yratio - M_PI_2;
                        Vec4 point = {
                            cos(xangle)*cos(yangle) * s.radius + s.center[0],
                            sin(xangle)*cos(yangle) * s.radius + s.center[1],
                            sin(yangle) * s.radius + s.center[2],
                            1
                        };
                        TriMesh::Vertex v;
                        v.position = point;
                        v.texCoord = { xratio, yratio };
                        v.color = o.color;
                        v.entityIndex = o.index;
                        vs[i][j] = v;
                    }
                }
                for (int i = 1; i < m; i++) {
                    int previ = i == 0 ? m - 1 : i - 1;
                    for (int j = 0; j < n; j++) {
                        int prevj = j == 0 ? n - 1 : j - 1;
                        mesh.addIsolatedTriangle(vs[i][j], vs[i][prevj], vs[previ][prevj]);
                        mesh.addIsolatedTriangle(vs[i][j], vs[previ][prevj], vs[previ][j]);
                    }
                }
            }
        }



        void Discretize(TriMesh & mesh, const SpatialProjectedPolygon & spp, const DiscretizeOptions & o){
            std::vector<Vec3> cs(spp.corners.size());
            for (int i = 0; i < spp.corners.size(); i++){
                InfiniteLine3 line(spp.projectionCenter, spp.corners[i] - spp.projectionCenter);
                cs[i] = IntersectionOfLineAndPlane(line, spp.plane).position;
            }
            std::vector<TriMesh::VertHandle> vhandles(cs.size());
            for (int i = 0; i < cs.size(); i++){
                TriMesh::Vertex v;
                v.position = Concat(cs[i], 1.0);
                v.normal = spp.plane.normal;
                v.color = o.color;
                v.entityIndex = o.index;
                vhandles[i] = mesh.addVertex(v);
            }
            mesh.addPolygon(vhandles);
        }



 
    }
}