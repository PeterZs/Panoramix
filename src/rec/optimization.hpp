#ifndef PANORAMIX_REC_OPTIMIZATION_HPP
#define PANORAMIX_REC_OPTIMIZATION_HPP
 
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <unsupported/Eigen/NonLinearOptimization>

#include "../deriv/derivative.hpp"

#include "../core/basic_types.hpp"
#include "../core/graphical_model.hpp"
#include "../core/utilities.hpp"


namespace panoramix {
    namespace rec {

        template <class T>
        class DisableableExpression {
        public:
            inline DisableableExpression() : _enabled(nullptr) {}
            inline DisableableExpression(const deriv::Expression<T> & rawExpr)
                : _enabled(std::make_shared<bool>(true)) {
                auto enabledExpr = deriv::composeFunction(*rawExpr.g(), [this]()
                    -> double {return *_enabled ? 1.0 : -1.0; });
                _expr = deriv::cwiseSelect(enabledExpr, rawExpr, 0.0);
            }

            inline deriv::Expression<T> expression() const { return _expr; }
            inline operator deriv::Expression<T>() const { return _expr; }
            inline void setEnabled(bool b) { *_enabled = b; }
            inline void enable() { *_enabled = true; }
            inline void disable() { *_enabled = false; }

        private:
            deriv::Expression<T> _expr;
            std::shared_ptr<bool> _enabled;
        };

        using EHandleTable = std::vector<deriv::EHandle>;

        template <class T>
        class OptimizibleExpression {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        public:
            inline OptimizibleExpression() {}
            inline explicit OptimizibleExpression(const T & d, deriv::ExpressionGraph & graph)
                : _data(std::make_shared<T>(d)), _frozen(false) {
                _lastChange = deriv::common::FillWithScalar(_lastChange, 0.0);
                T * dataPtr = _data.get();
                _expr = deriv::composeFunction(graph, [dataPtr]() ->T {
                    return *dataPtr;
                });
            }

            void registerHandleTable(EHandleTable & table) {
                table.push_back(_expr.handle());
                _positionInHandleTable = table.size() - 1;
            }

            void getDerivative(const EHandleTable & derivTable) {
                auto derivHandle = derivTable[_positionInHandleTable];
                if (derivHandle.isValid())
                    _dexpr = _expr.g()->asDerived<T>(derivHandle);
                else
                    _dexpr = deriv::DerivativeExpression<T>(); // invalidate
            }

            void optimizeData(double delta, double momentum, const EHandleTable & table){
                if (_dexpr.isValid() && !_frozen){
                    auto grad = _dexpr.executeHandlesRange(table.begin(), table.end());
                    _lastChange = (-grad * (1 - momentum) + _lastChange * momentum) * delta;
                    *_data += _lastChange;
                }
            }

            inline void freeze() { _frozen = true; }
            inline void unFreeze() { _frozen = false; }

            inline deriv::Expression<T> expression() const { return _expr; }
            inline deriv::DerivativeExpression<T> derivativeExpression() const { return _dexpr; }

        private:
            deriv::Expression<T> _expr;
            deriv::DerivativeExpression<T> _dexpr;
            int _positionInHandleTable;
            std::shared_ptr<T> _data;
            T _lastChange;
            bool _frozen;
        };

        


        

    }
}
 
#endif