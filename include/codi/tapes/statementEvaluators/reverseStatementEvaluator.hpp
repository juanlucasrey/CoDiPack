#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>

#include "../../aux/macros.hpp"
#include "../../aux/memberStore.hpp"
#include "../../aux/exceptions.hpp"
#include "../../expressions/activeType.hpp"
#include "statementEvaluatorInterface.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  template<typename _Real>
  struct ReverseStatementEvaluator : public StatementEvaluatorInterface<_Real> {

      using Real = CODI_DECLARE_DEFAULT(_Real, double);

    public:

      /*******************************************************************************
       * Section: Start of interface definition
       *
       */
      using Handle = void*;

      template<typename Tape, typename ... Args>
      static Real callForward(Handle const& h, Args&& ... args) {
        CODI_UNUSED(h, args...);

        CODI_EXCEPTION("ReverseStatementEvaluator does not support forward evaluation calls.");

        return Real();
      }

      template<typename Tape, typename ... Args>
      static Real callPrimal(Handle const& h, Args&& ... args) {
        CODI_UNUSED(h, args...);

        CODI_EXCEPTION("ReverseStatementEvaluator does not support primal evaluation calls.");

        return Real();
      }

      template<typename Tape, typename ... Args>
      static void callReverse(Handle const& h, Args&& ... args) {
        HandleTyped<Tape> func = (HandleTyped<Tape>)h;

        func(std::forward<Args>(args)...);
      }

      template<typename Tape, typename Generator, typename Expr>
      static Handle createHandle() {
        return (Handle*)Generator::template statementEvaluateReverse<Expr>;
      }

    protected:

      template<typename Tape>
      using HandleTyped = decltype(&Tape::template statementEvaluateReverse<ActiveType<Tape>>);

  };
}