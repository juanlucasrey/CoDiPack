#pragma once

#include <complex>
#include <vector>

#include "../../aux/macros.hpp"
#include "../../config.h"
#include "../../expressions/lhsExpressionInterface.hpp"
#include "../../tapes/aux/vectorAccessInterface.hpp"
#include "../../traits/computationTraits.hpp"
#include "../../traits/expressionTraits.hpp"
#include "../../traits/realTraits.hpp"

/** \copydoc codi::Namespace */
namespace codi {

  /**
   * @brief Generalized wrapper for VectorAccessInterface for aggregated data types,
   * e.g. std::complex<codi::RealReverse>.
   *
   * This wrapper is instantiated by AggregatedTypeVectorAccessWrapperFactory. It can be specialized for arbitrary
   * types that consist of CoDiPack types.
   *
   * This class helps to write generalized external function, that handles aggregated data types. E.g. for
   * std::complex<codi::RealReverse> the primal value as well as the adjoint value are std::complex<double> and the
   * identifier type is std::complex<int>. Since this can be different for other types like vectors or matrices, this
   * wrapper can be used to write generalized code, that works for arbitrary aggregated types. See
   * RealTraits::DataExtraction for a generalized access to the primal and identifier data of aggregated types.
   *
   * Here is an example for a generalized external function routine
   * (documentation/examples/Example_20_Aggregated_active_type_handling.cpp): \snippet
   * examples/Example_20_Aggregated_active_type_handling.cpp Typed external function
   *
   * In general all implementations of the wrapper will forward all functions calls to the VectorAccessInterface of
   * the underlying tape. For each active type in the structure the corresponding operation should be performed.
   *
   * Implementations can use AggregatedTypeVectorAccessWrapperBase which implements most of the functions from
   * VectorAccessInterface.
   *
   * @tparam _Type  An arbitrary type which consists of multiple CoDiPack types.
   */
  template<typename _Type, typename = void>
  struct AggregatedTypeVectorAccessWrapper : public VectorAccessInterface<CODI_ANY, CODI_ANY> {
      static_assert(false && std::is_void<_Type>::value,
                    "Instantiation of unspecialized AggregatedTypeVectorAccessWrapper.");

      using Type = CODI_DD(_Type, CODI_ANY);  ///< See AggregatedTypeVectorAccessWrapperBase.
  };

  /// @brief Implements all methods from AggregatedTypeVectorAccessWrapper, that can be implemented with combinations of
  /// other methods.
  ///
  /// @tparam _Real  Primal value type of the combined type.
  /// @tparam _Identifier  Identifier type type of the combined type.
  /// @tparam _InnerInterface  The VectorAccessInterface of the underlying tape.
  template<typename _Real, typename _Identifier, typename _InnerInterface>
  struct AggregatedTypeVectorAccessWrapperBase : public VectorAccessInterface<_Real, _Identifier> {
    public:

      using Real = CODI_DD(_Real, CODI_ANY);              ///< See RealTraits::DataExtraction::Real.
      using Identifier = CODI_DD(_Identifier, CODI_ANY);  ///< See RealTraits::DataExtraction::Identifier.

      using InnerInterface = CODI_DD(
          _InnerInterface, CODI_T(VectorAccessInterface<double, int>));  ///< See AggregatedTypeVectorAccessWrapperBase.

    protected:

      InnerInterface& innerInterface;  ///< Reference to the accessor of the underlying tape.

      std::vector<Real> lhs;  ///< Temporary storage for indirect adjoint or tangent updates.

    public:

      /// Constructor
      AggregatedTypeVectorAccessWrapperBase(InnerInterface* innerInterface)
          : innerInterface(*innerInterface), lhs(innerInterface->getVectorSize()) {}

      /*******************************************************************************/
      /// @name Misc

      /// \copydoc VectorAccessInterface::getVectorSize()
      size_t getVectorSize() const {
        return innerInterface.getVectorSize();
      }

      /// \copydoc VectorAccessInterface::isLhsZero()
      bool isLhsZero() {
        bool isZero = true;

        for (size_t curDim = 0; isZero && curDim < lhs.size(); curDim += 1) {
          isZero &= RealTraits::isTotalZero(lhs[curDim]);
        }

        return isZero;
      }

      /*******************************************************************************/
      /// @name Indirect adjoint access

      /// \copydoc VectorAccessInterface::setLhsAdjoint()
      void setLhsAdjoint(Identifier const& index) {
        getAdjointVec(index, lhs.data());
        this->resetAdjointVec(index);
      }

      /// \copydoc VectorAccessInterface::updateAdjointWithLhs()
      void updateAdjointWithLhs(Identifier const& index, Real const& jacobi) {
        for (size_t curDim = 0; curDim < lhs.size(); curDim += 1) {
          Real update = jacobi * lhs[curDim];
          this->updateAdjoint(index, curDim, update);
        }
      }

      /*******************************************************************************/
      /// @name Indirect tangent access

      /// \copydoc VectorAccessInterface::setLhsTangent()
      void setLhsTangent(Identifier const& index) {
        updateAdjointVec(index, lhs.data());

        for (size_t curDim = 0; curDim < lhs.size(); curDim += 1) {
          lhs[curDim] = Real();
        }
      }

      /// \copydoc VectorAccessInterface::updateTangentWithLhs()
      void updateTangentWithLhs(Identifier const& index, Real const& jacobi) {
        for (size_t curDim = 0; curDim < lhs.size(); curDim += 1) {
          lhs[curDim] += jacobi * this->getAdjoint(index, curDim);
        }
      }

      /*******************************************************************************/
      /// @name Direct adjoint access

      /// \copydoc VectorAccessInterface::getAdjointVec()
      void getAdjointVec(Identifier const& index, Real* const vec) {
        for (size_t curDim = 0; curDim < lhs.size(); curDim += 1) {
          vec[curDim] = this->getAdjoint(index, curDim);
        }
      }

      /// \copydoc VectorAccessInterface::updateAdjointVec()
      void updateAdjointVec(Identifier const& index, Real const* const vec) {
        for (size_t curDim = 0; curDim < lhs.size(); curDim += 1) {
          this->updateAdjoint(index, curDim, vec[curDim]);
        }
      }

      /*******************************************************************************/
      /// @name Primal access

      /// \copydoc VectorAccessInterface::hasPrimals()
      bool hasPrimals() {
        return innerInterface.hasPrimals();
      }
  };

  /// @brief Factory for the creation of AggregatedTypeVectorAccessWrapper instances.
  ///
  /// This factory is specialized for CoDiPack types to return the provided interface, thus removing the overhead of
  /// a wrapped interface.
  ///
  /// User can specialize this factory if the default construction of AggregatedTypeVectorAccessWrapper needs to be
  /// specialized.
  ///
  /// @tparam _Type See AggregatedTypeVectorAccessWrapper.
  template<typename _Type, typename = void>
  struct AggregatedTypeVectorAccessWrapperFactory {
    public:
      using Type = CODI_DD(_Type, CODI_ANY);  ///< See AggregatedTypeVectorAccessWrapperBase.

      using RType = AggregatedTypeVectorAccessWrapper<Type>;  ///< Which instances this factory creates.

      /// Instantiate a AggregatedTypeVectorAccessWrapper class.
      ///
      /// @param access  The vector access interface from underlying tape.
      template<typename Real, typename Identifier>
      static RType* create(VectorAccessInterface<Real, Identifier>* access) {
        return new RType(access);
      }

      /// Delete the AggregatedTypeVectorAccessWrapper instance create by the crate method.
      static void destroy(RType* access) {
        delete access;
      }
  };

#ifndef DOXYGEN_DISABLE
  /// Specialization of AggregatedTypeVectorAccessWrapper for std::complex.
  ///
  /// @tparam The nested type of the complex data type.
  template<typename _InnerType>
  struct AggregatedTypeVectorAccessWrapper<std::complex<_InnerType>>
      : public AggregatedTypeVectorAccessWrapperBase<
            std::complex<typename _InnerType::Real>, std::complex<typename _InnerType::Identifier>,
            VectorAccessInterface<typename _InnerType::Real, typename _InnerType::Identifier>> {
    public:

      using InnerType = CODI_DD(
          _InnerType,
          CODI_T(LhsExpressionInterface<double, double, CODI_ANY, CODI_ANY>));  ///< See
                                                                                ///< AggregatedTypeVectorAccessWrapper.
      using Type = std::complex<InnerType>;  ///< See AggregatedTypeVectorAccessWrapper.

      using InnerInterface = VectorAccessInterface<
          typename InnerType::Real,
          typename InnerType::Identifier>;  ///< See AggregatedTypeVectorAccessWrapperBase::InnerInterface.

      using Real = std::complex<typename InnerType::Real>;              ///< See RealTraits::DataExtraction::Real.
      using Identifier = std::complex<typename InnerType::Identifier>;  ///< See RealTraits::DataExtraction::Real.

      using Base =
          AggregatedTypeVectorAccessWrapperBase<Real, Identifier, InnerInterface>;  ///< Base class abbreviation.

      /// Constructor
      AggregatedTypeVectorAccessWrapper(InnerInterface* innerInterface) : Base(innerInterface) {}

      /*******************************************************************************/
      /// @name Direct adjoint access

      /// \copydoc VectorAccessInterface::resetAdjoint()
      void resetAdjoint(Identifier const& index, size_t dim) {
        Base::innerInterface.resetAdjoint(std::real(index), dim);
        Base::innerInterface.resetAdjoint(std::imag(index), dim);
      }

      /// \copydoc VectorAccessInterface::resetAdjointVec()
      void resetAdjointVec(Identifier const& index) {
        Base::innerInterface.resetAdjointVec(std::real(index));
        Base::innerInterface.resetAdjointVec(std::imag(index));
      }

      /// \copydoc VectorAccessInterface::getAdjoint()
      Real getAdjoint(Identifier const& index, size_t dim) {
        return Real(Base::innerInterface.getAdjoint(std::real(index), dim),
                    Base::innerInterface.getAdjoint(std::imag(index), dim));
      }

      /// \copydoc VectorAccessInterface::updateAdjoint()
      void updateAdjoint(Identifier const& index, size_t dim, Real const& adjoint) {
        Base::innerInterface.updateAdjoint(std::real(index), dim, std::real(adjoint));
        Base::innerInterface.updateAdjoint(std::imag(index), dim, std::imag(adjoint));
      }

      /*******************************************************************************/
      /// @name Primal access

      /// \copydoc VectorAccessInterface::setPrimal()
      void setPrimal(Identifier const& index, Real const& primal) {
        Base::innerInterface.setPrimal(std::real(index), std::real(primal));
        Base::innerInterface.setPrimal(std::imag(index), std::imag(primal));
      }

      /// \copydoc VectorAccessInterface::getPrimal()
      Real getPrimal(Identifier const& index) {
        return Real(Base::innerInterface.getPrimal(std::real(index)), Base::innerInterface.getPrimal(std::imag(index)));
      }
  };

  /// Specialization of AggregatedTypeVectorAccessWrapperFactory for CoDiPack active types.
  ///
  /// @tparam _Type  A CoDiPack active type.
  template<typename _Type>
  struct AggregatedTypeVectorAccessWrapperFactory<_Type, ExpressionTraits::EnableIfLhsExpression<_Type>> {
    public:
      using Type = CODI_DD(
          _Type,
          CODI_T(LhsExpressionInterface<double, int, CODI_ANY, CODI_ANY>));  ///< See
                                                                             ///< AggregatedTypeVectorAccessWrapperBase.

      using RType = VectorAccessInterface<typename Type::Real, typename Type::Identifier>;

      /// \copydoc AggregatedTypeVectorAccessWrapperFactory::create()
      static RType* create(RType* access) {
        return access;
      }

      /// \copydoc AggregatedTypeVectorAccessWrapperFactory::destroy()
      static void destroy(RType* access) {
        // Do nothing
      }
  };
#endif
}
