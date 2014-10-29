
#include "utilities.hpp"
#include "cameras.hpp"

namespace panoramix {
    namespace core {

        PerspectiveCamera::PerspectiveCamera(int w, int h, double focal, const Vec3 & eye,
            const Vec3 & center, const Vec3 & up, double near, double far)
            : _screenW(w), _screenH(h), _focal(focal), _eye(eye), _center(center), _up(up), _near(near), _far(far) {
            updateMatrices();
        }

        void PerspectiveCamera::updateMatrices() {
            _viewMatrix = MakeMat4LookAt(_eye, _center, _up);

            double verticalViewAngle = atan(_screenH / 2.0 / _focal) * 2;
            double aspect = double(_screenW) / double(_screenH);
            _projectionMatrix = MakeMat4Perspective(verticalViewAngle, aspect, _near, _far);

            _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
            _viewProjectionMatrixInv = _viewProjectionMatrix.inv();
        }

        Vec2 PerspectiveCamera::screenProjection(const Vec3 & p3) const {
            Vec4 p4(p3(0), p3(1), p3(2), 1);
            Vec4 position = _viewProjectionMatrix * p4;
            double xratio = position(0) / position(3) / 2;
            double yratio = position(1) / position(3) / 2;
            double x = (xratio + 0.5) * _screenW;
            double y = _screenH - (yratio + 0.5) * _screenH;
            return Vec2(x, y);
        }

        bool PerspectiveCamera::isVisibleOnScreen(const Vec3 & p3d) const {
            Vec4 p4(p3d(0), p3d(1), p3d(2), 1);
            Vec4 position = _viewProjectionMatrix * p4;
            return position(3) > 0 && position(2) > 0;
        }

        HPoint2 PerspectiveCamera::screenProjectionInHPoint(const Vec3 & p3) const {
            Vec4 p4(p3(0), p3(1), p3(2), 1);
            Vec4 position = _viewProjectionMatrix * p4;
            double xratio = position(0) / 2;
            double yratio = position(1) / 2;
            double zratio = position(3);

            double x = (xratio + 0.5 * zratio) * _screenW;
            double y = _screenH * zratio - (yratio + 0.5 * zratio) * _screenH;
            return HPoint2({ x, y }, zratio);
        }

        Vec3 PerspectiveCamera::spatialDirection(const Vec2 & p2d) const {
            double xratio = (p2d(0) / _screenW - 0.5) * 2;
            double yratio = ((_screenH - p2d(1)) / _screenH - 0.5) * 2;
            Vec4 position(xratio, yratio, 1, 1);
            Vec4 realPosition = _viewProjectionMatrixInv * position;
            return Vec3(realPosition(0) / realPosition(3),
                realPosition(1) / realPosition(3),
                realPosition(2) / realPosition(3));
        }

        void PerspectiveCamera::resizeScreen(const Size & sz, bool updateMat) {
            if (_screenH == sz.height && _screenW == sz.width)
                return;
            _screenH = sz.height;
            _screenW = sz.width;
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::setFocal(double f, bool updateMat) {
            if (f == _focal)
                return;
            _focal = f;
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::setEye(const Vec3 & e, bool updateMat) {
            if (_eye == e)
                return;
            _eye = e;
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::setCenter(const Vec3 & c, bool updateMat) {
            if (_center == c)
                return;
            _center = c;
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::setUp(const Vec3 & up, bool updateMat) {
            if (_up == up)
                return;
            _up = up;
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::setNearAndFarPlanes(double near, double far, bool updateMat) {
            if (_near == near && _far == far)
                return;
            _near = near;
            _far = far;
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::focusOn(const Sphere3 & target, bool updateMat) {
            _center = target.center;
            auto eyedirection = _eye - _center;
            eyedirection = eyedirection / core::norm(eyedirection) * target.radius * 0.8;
            _eye = _center + eyedirection;
            _near = BoundBetween(norm(target.center - _eye) - target.radius - 1.0, 1e-3, 1e3);
            _far = BoundBetween(norm(target.center - _eye) + target.radius + 1.0, 1e-3, 1e3);
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::translate(const Vec3 & t, const Sphere3 & target, bool updateMat){
            _eye += t;
            _center += t;
            _near = BoundBetween(norm(target.center - _eye) - target.radius - 1.0, 1e-3, 1e3);
            _far = BoundBetween(norm(target.center - _eye) + target.radius + 1.0, 1e-3, 1e3);
            if (updateMat)
                updateMatrices();
        }

        void PerspectiveCamera::moveEyeWithCenterFixed(const Vec3 & t, const Sphere3 & target, bool distanceFixed, bool updateMat){
            double dist = norm(_eye - _center);
            _eye += t;
            if (distanceFixed){
                _eye = normalize(_eye - _center) * dist + _center;
            }
            _near = BoundBetween(norm(target.center - _eye) - target.radius - 1.0, 1e-3, 1e3);
            _far = BoundBetween(norm(target.center - _eye) + target.radius + 1.0, 1e-3, 1e3);
            if (updateMat)
                updateMatrices();
        }





        PanoramicCamera::PanoramicCamera(double focal, const Vec3 & eye,
            const Vec3 & center, const Vec3 & up)
            : _focal(focal), _eye(eye), _center(center), _up(up) {
            _xaxis = (_center - _eye); _xaxis /= core::norm(_xaxis);
            _yaxis = _up.cross(_xaxis); _yaxis /= core::norm(_yaxis);
            _zaxis = _xaxis.cross(_yaxis);
        }

        Vec2 PanoramicCamera::screenProjection(const Vec3 & p3) const {
            double xx = p3.dot(_xaxis);
            double yy = p3.dot(_yaxis);
            double zz = p3.dot(_zaxis);
            GeoCoord pg = core::Vec3(xx, yy, zz);
            auto sz = screenSize();
            double x = (pg.longitude + M_PI) / 2.0 / M_PI * sz.width;
            double y = (pg.latitude + M_PI_2) / M_PI * sz.height;
            return Vec2(x, y);
        }

        Vec3 PanoramicCamera::spatialDirection(const Vec2 & p2d) const {
            auto sz = screenSize();
            double longi = p2d(0) / double(sz.width) * 2 * M_PI - M_PI;
            double lati = p2d(1) / double(sz.height) * M_PI - M_PI_2;
            Vec3 dd = (GeoCoord(longi, lati).toVector());
            return dd(0) * _xaxis + dd(1) * _yaxis + dd(2) * _zaxis;
        }


    }
}