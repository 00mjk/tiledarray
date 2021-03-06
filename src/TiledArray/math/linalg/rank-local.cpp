/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2020 Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Eduard Valeyev
 *
 *  lapack.cpp
 *  Created:    16 October, 2020
 *
 */

#include <TiledArray/math/blas.h>
#include <TiledArray/math/lapack.h>
#include <TiledArray/math/linalg/rank-local.h>

template <class F, typename... Args>
inline int ta_lapack_fortran_call(F f, Args... args) {
  lapack_int info;
  auto ptr = [](auto&& a) {
    using T = std::remove_reference_t<decltype(a)>;
    if constexpr (std::is_pointer_v<T>)
      return a;
    else
      return &a;
  };
  f(ptr(args)..., &info);
  return info;
}

#define TA_LAPACK_ERROR(F) throw std::runtime_error("lapack::" #F " failed")

#define TA_LAPACK_FORTRAN_CALL(F, ARGS...) \
  ((ta_lapack_fortran_call(F, ARGS) == 0) || (TA_LAPACK_ERROR(F), 0))

/// \brief Invokes the Fortran LAPACK API

/// \warning TA_LAPACK_FORTRAN(fn,args) can be called only from template
/// context, with `T` defining the element type
#define TA_LAPACK_FORTRAN(name, args...)                    \
  {                                                         \
    using numeric_type = T;                                 \
    if constexpr (std::is_same_v<numeric_type, double>)     \
      TA_LAPACK_FORTRAN_CALL(d##name, args);                \
    else if constexpr (std::is_same_v<numeric_type, float>) \
      TA_LAPACK_FORTRAN_CALL(s##name, args);                \
    else                                                    \
      std::abort();                                         \
  }

/// TA_LAPACK(fn,args) invoked lapack::fn directly and checks the return value
#define TA_LAPACK(name, args...) \
  ((::lapack::name(args) == 0) || (TA_LAPACK_ERROR(name), 0))

namespace TiledArray::math::linalg::rank_local {

using integer = math::blas::integer;

template <typename T>
void cholesky(Matrix<T>& A) {
  auto uplo = lapack::Uplo::Lower;
  integer n = A.rows();
  auto* a = A.data();
  integer lda = n;
  TA_LAPACK(potrf, uplo, n, a, lda);
}

template <typename T>
void cholesky_linv(Matrix<T>& A) {
  auto uplo = lapack::Uplo::Lower;
  auto diag = lapack::Diag::NonUnit;
  integer n = A.rows();
  auto* l = A.data();
  integer lda = n;
  TA_LAPACK(trtri, uplo, diag, n, l, lda);
}

template <typename T>
void cholesky_solve(Matrix<T>& A, Matrix<T>& X) {
  auto uplo = lapack::Uplo::Lower;
  integer n = A.rows();
  integer nrhs = X.cols();
  auto* a = A.data();
  auto* b = X.data();
  integer lda = n;
  integer ldb = n;
  TA_LAPACK(posv, uplo, n, nrhs, a, lda, b, ldb);
}

template <typename T>
void cholesky_lsolve(Op transpose, Matrix<T>& A, Matrix<T>& X) {
  auto uplo = lapack::Uplo::Lower;
  auto diag = lapack::Diag::NonUnit;
  integer n = A.rows();
  integer nrhs = X.cols();
  auto* a = A.data();
  auto* b = X.data();
  integer lda = n;
  integer ldb = n;
  TA_LAPACK(trtrs, uplo, transpose, diag, n, nrhs, a, lda, b, ldb);
}

template <typename T>
void heig(Matrix<T>& A, std::vector<T>& W) {
  auto jobz = lapack::Job::Vec;
  auto uplo = lapack::Uplo::Lower;
  integer n = A.rows();
  T* a = A.data();
  integer lda = A.rows();
  W.resize(n);
  T* w = W.data();
  TA_LAPACK(syev, jobz, uplo, n, a, lda, w);
}

template <typename T>
void heig(Matrix<T>& A, Matrix<T>& B, std::vector<T>& W) {
  integer itype = 1;
  auto jobz = lapack::Job::Vec;
  auto uplo = lapack::Uplo::Lower;
  integer n = A.rows();
  T* a = A.data();
  integer lda = A.rows();
  T* b = B.data();
  integer ldb = B.rows();
  W.resize(n);
  T* w = W.data();
  TA_LAPACK(sygv, itype, jobz, uplo, n, a, lda, b, ldb, w);
}

template <typename T>
void svd(Matrix<T>& A, std::vector<T>& S, Matrix<T>* U, Matrix<T>* VT) {
  integer m = A.rows();
  integer n = A.cols();
  T* a = A.data();
  integer lda = A.rows();

  S.resize(std::min(m, n));
  T* s = S.data();

  auto jobu = lapack::Job::NoVec;
  T* u = nullptr;
  integer ldu = m;
  if (U) {
    jobu = lapack::Job::AllVec;
    U->resize(m, n);
    u = U->data();
    ldu = U->rows();
  }

  auto jobvt = lapack::Job::NoVec;
  T* vt = nullptr;
  integer ldvt = n;
  if (VT) {
    jobvt = lapack::Job::AllVec;
    VT->resize(n, m);
    vt = VT->data();
    ldvt = VT->rows();
  }

  TA_LAPACK(gesvd, jobu, jobvt, m, n, a, lda, s, u, ldu, vt, ldvt);
}

template <typename T>
void lu_solve(Matrix<T>& A, Matrix<T>& B) {
  integer n = A.rows();
  integer nrhs = B.cols();
  T* a = A.data();
  integer lda = A.rows();
  T* b = B.data();
  integer ldb = B.rows();
  std::vector<integer> ipiv(n);
  TA_LAPACK(gesv, n, nrhs, a, lda, ipiv.data(), b, ldb);
}

template <typename T>
void lu_inv(Matrix<T>& A) {
  integer n = A.rows();
  T* a = A.data();
  integer lda = A.rows();
  std::vector<integer> ipiv(n);
  TA_LAPACK(getrf, n, n, a, lda, ipiv.data());
  TA_LAPACK(getri, n, a, lda, ipiv.data());
}

#define TA_LAPACK_EXPLICIT(MATRIX, VECTOR)               \
  template void cholesky(MATRIX&);                       \
  template void cholesky_linv(MATRIX&);                  \
  template void cholesky_solve(MATRIX&, MATRIX&);        \
  template void cholesky_lsolve(Op, MATRIX&, MATRIX&);   \
  template void heig(MATRIX&, VECTOR&);                  \
  template void heig(MATRIX&, MATRIX&, VECTOR&);         \
  template void svd(MATRIX&, VECTOR&, MATRIX*, MATRIX*); \
  template void lu_solve(MATRIX&, MATRIX&);              \
  template void lu_inv(MATRIX&);

TA_LAPACK_EXPLICIT(Matrix<double>, std::vector<double>);
TA_LAPACK_EXPLICIT(Matrix<float>, std::vector<float>);

}  // namespace TiledArray::math::linalg::rank_local
