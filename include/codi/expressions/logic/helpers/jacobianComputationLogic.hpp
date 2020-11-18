#pragma once

#include <type_traits>
#include <utility>

#include "../../../aux/macros.hpp"
#include "../../../config.h"
#include "../traversalLogic.hpp"
#include "../../../traits/expressionTraits.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  template<typename _Real, typename _Impl>
  struct JacobianComputationLogic : public TraversalLogic<_Impl> {
    public:

      using Real = CODI_DECLARE_DEFAULT(_Real, double);
      using Impl = CODI_DECLARE_DEFAULT(_Impl, CODI_TEMPLATE(TraversalLogic<CODI_ANY>));

      /*******************************************************************************
       * Section: Methods expected in the child class.
       *
       * Description: TODO
       *
       */

      /// TODO
      template<typename Node, typename ... Args>
      void handleJacobianOnActive(Node const& node, Real jacobian, Args&& ... args);

      /*******************************************************************************
       * Section: Implementation of Jacobian computation
       *
       * Description: TODO
       */

      template<typename Node, typename ... Args>
      CODI_INLINE enableIfLhsExpression<Node> term(Node const& node, Real jacobian, Args&& ... args) {
        cast().handleJacobianOnActive(node, jacobian, std::forward<Args>(args)...);
      }
      using TraversalLogic<Impl>::term;

      template<size_t LeafNumber, typename Leaf, typename Root, typename ... Args>
      CODI_INLINE void link(Leaf const& leaf, Root const& root, Real const& jacobian, Args&& ... args) {

        Real curJacobian = root.template getJacobian<LeafNumber>() * jacobian;

        cast().toNode(leaf, curJacobian, std::forward<Args>(args)...);
      }

    private:

      CODI_INLINE Impl& cast() {
        return static_cast<Impl&>(*this);
      }

  };
}
