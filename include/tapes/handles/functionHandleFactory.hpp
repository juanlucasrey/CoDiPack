/*
 * CoDiPack, a Code Differentiation Package
 *
 * Copyright (C) 2015-2017 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (codi@scicomp.uni-kl.de)
 *
 * Lead developers: Max Sagebaum, Tim Albring (SciComp, TU Kaiserslautern)
 *
 * This file is part of CoDiPack (http://www.scicomp.uni-kl.de/software/codi).
 *
 * CoDiPack is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * CoDiPack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with CoDiPack.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Max Sagebaum, Tim Albring, (SciComp, TU Kaiserslautern)
 */

#pragma once

#include <utility>

#include "../../configure.h"
#include "../../evaluateDefinitions.hpp"
#include "../../tapeTypes.hpp"
#include "../../typeTraits.hpp"

#include "handleFactoryInterface.hpp"

/**
 * @brief Global namespace for CoDiPack - Code Differentiation Package
 */
namespace codi {

  /**
   * @brief A factory for function handles, that just uses function objects.
   *
   * The static data of the expression is curried into the function call and this function
   * pointer is returned as the handle.
   */
  template<typename ReverseTapeTypes>
  struct FunctionHandleFactory
    // final : public HandleFactoryInterface</* handle type */>
  {

    /**
     * @brief The type for handle that is used by this factory
     *
     * The handle is just a function pointer that performs the reverse evaluation
     */
    typedef typename EvaluateDefinitions<ReverseTapeTypes>::AdjointFunc Handle;

    /**
     * @brief Create the handle for the given tape and the given expression.
     *
     * @tparam Expr  The expression that performs the evaluation of the reverse AD operations.
     * @tparam Tape  The tape that is performing the reverse AD evaluation.
     */
    template<typename Expr, typename Tape>
    static CODI_INLINE Handle createHandle() {

      return &Tape::template curryEvaluateHandle<Expr>;
    }

    /**
     * @brief The evaluation of the primal handle, that was created by this factory.
     *
     * @param[in]           handle  The handle the was generated by this factory and is called with the arguments.
     * @param[in,out]         args  The other arguments for the function.
     *
     * @tparam Tape  The tape that is performing the reverse AD evaluation.
     * @tparam Args  The arguments for the function.
     */
    template<typename Tape, typename ... Args>
    static CODI_INLINE typename Tape::Real callPrimalHandle(Handle handle, Args&& ... args) {
      CODI_UNUSED(handle);

      std::cerr << "Error: Primal handles are not supported by this handle factory." << std::endl;
      exit(-1);
      return typename Tape::Real();
    }

    /**
     * @brief The evaluation of the handle, that was created by this factory.
     *
     * @param[in]           handle  The handle the was generated by this factory and is called with the arguments.
     * @param[in,out]         args  The other arguments for the function.
     *
     * @tparam Tape  The tape that is performing the reverse AD evaluation.
     * @tparam Args  The arguments for the function.
     */
    template<typename Tape, typename ... Args>
    static CODI_INLINE void callHandle(Handle handle, Args&& ... args) {

      handle(std::forward<Args>(args)...);
    }

  };
}
