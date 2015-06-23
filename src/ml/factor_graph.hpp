#ifndef PANORAMIX_ML_FACTOR_GRAPH_HPP
#define PANORAMIX_ML_FACTOR_GRAPH_HPP

#include <type_traits>

#include "../core/iterators.hpp"
#include "../core/basic_types.hpp"
#include "../core/homo_graph.hpp"
 
namespace panoramix {
    namespace ml {

        class FactorGraph {
        public:
            using FactorCategoryId = int;
            using VarCategoryId = int;
            using CostFunction = std::function<double(const int * varlabels, size_t nvar, 
                FactorCategoryId fcid, void * givenData)>;

            struct FactorCategory {
                CostFunction costs;
                double c_alpha;
            };
            struct VarCategory {
                size_t nlabels;
                double c_i;
            };

            using Topology = core::HomogeneousGraph0x<VarCategoryId, FactorCategoryId>;
            using VarHandle = core::HandleOfTypeAtLevel<Topology, 0>;
            using FactorHandle = core::HandleOfTypeAtLevel<Topology, 1>;
            using ResultTable = core::HandledTable<VarHandle, int>;

            using SimpleCallbackFunction = std::function<bool(int epoch, double energy)>;
            using CallbackFunction = std::function<bool(int epoch, double energy, double denergy, const ResultTable & results)>;

        public:
            void reserveVarCategories(size_t cap) { _varCategories.reserve(cap); }
            void reserveFactorCategories(size_t cap) { _factorCategories.reserve(cap); }

            VarCategoryId addVarCategory(VarCategory && vc) { 
                _varCategories.push_back(std::move(vc)); return _varCategories.size() - 1;
            }
            VarCategoryId addVarCategory(const VarCategory & vc) { 
                _varCategories.push_back(vc); return _varCategories.size() - 1; 
            }
            VarCategoryId addVarCategory(size_t nlabels, double c_i) {
                return addVarCategory(VarCategory{ nlabels, c_i });
            }

            const VarCategory & varCategory(VarCategoryId vid) const { return _varCategories.at(vid); }
            VarCategory & varCategory(VarCategoryId vid) { return _varCategories.at(vid); }

            FactorCategoryId addFactorCategory(FactorCategory && fc) {
                _factorCategories.push_back(std::move(fc)); return _factorCategories.size() - 1; 
            }
            FactorCategoryId addFactorCategory(const FactorCategory & fc) {
                _factorCategories.push_back(fc); return _factorCategories.size() - 1; 
            }

            const FactorCategory & factorCategory(FactorCategoryId fid) const { return _factorCategories.at(fid); }
            FactorCategory & factorCategory(FactorCategoryId fid) { return _factorCategories.at(fid); }

            void reserveVars(size_t cap) { _graph.internalElements<0>().reserve(cap); }
            void reserveFactors(size_t cap) { _graph.internalElements<1>().reserve(cap); }

            VarHandle addVar(VarCategoryId vc) { return _graph.add(vc); }
            const VarCategory & varCategory(VarHandle vh) const { return _varCategories.at(_graph.data(vh)); }
            VarCategory & varCategory(VarHandle vh) { return _varCategories.at(_graph.data(vh)); }

            FactorHandle addFactor(std::initializer_list<VarHandle> vhs, FactorCategoryId fc) { 
                return _graph.add<1>(vhs, fc); 
            }
            template <class IteratorT>
            FactorHandle addFactor(IteratorT vhsBegin, IteratorT vhsEnd, FactorCategoryId fc) { 
                return _graph.add<1>(vhsBegin, vhsEnd, fc); 
            }
            const FactorCategory & factorCategory(FactorHandle fh) const { return _factorCategories.at(_graph.data(fh)); }
            FactorCategory & factorCategory(FactorHandle fh) { return _factorCategories.at(_graph.data(fh)); }

            void clear() { _varCategories.clear(); _factorCategories.clear(); _graph.clear(); }
            bool valid() const;
            double energy(const ResultTable & labels, void * givenData = nullptr) const;

            // convex belief propagation
            ResultTable solve(int maxEpoch, int innerLoopNum = 10, 
                const CallbackFunction & callback = nullptr, void * givenData = nullptr) const;
            ResultTable solveWithSimpleCallback(int maxEpoch, int innerLoopNum,
                const SimpleCallbackFunction & callback, void * givenData = nullptr) const;

        private:
            std::vector<VarCategory> _varCategories;
            std::vector<FactorCategory> _factorCategories;
            Topology _graph;
        };




    }
}
 
#endif