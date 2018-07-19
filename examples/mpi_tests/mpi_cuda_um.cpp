/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2015  Virginia Tech
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
 *  Created by Chong Peng on 7/19/18.
 *
 */


#include <mpi.h>
#include <cuda_runtime.h>

#include <stdexcept>
#include <iostream>
#include <assert.h>

/**
 *  Test CUDA-aware MPI
 */

const std::size_t N = 100;

int main(int argc, char *argv[]) {

  MPI_Init(&argc, &argv);

  int mpi_size = -1;
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  if(mpi_size != 2 ){
    throw std::runtime_error("Must run this program with 2 mpi processes");
  }

  int mpi_rank = -1;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  // allocate host vector
  int* vector_um;
  cudaError_t cuda_error;
  cudaMallocManaged(&vector_um, sizeof(int)*N);
  assert(cuda_error == cudaSuccess);

  // initialize data on node 0
  if(mpi_rank == 0){

    for (std::size_t i = 0; i < N; i++){
      vector_um[i] = i;
    }

    // MPI SEND to NODE 1
    MPI_Send(vector_um, N, MPI_INT, 1, 100, MPI_COMM_WORLD);
  }
  // receive data on node 1
  else{

    MPI_Status status;
    MPI_Recv(vector_um, N, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);

    // verify the data
    for (std::size_t i = 0; i < N; i++){
      assert(vector_um[i] == i);
    }
  }

  cudaFree(vector_um);
  
  if( mpi_rank == 0){
    std::cout << "MPI Send & Recv SUCCESS on CUDA Unified memory.\n";
  }

  MPI_Finalize();
  return 0;
}
