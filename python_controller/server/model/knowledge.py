from typing import Dict

import model.prediction as prediction

import pandas as pd

class Column :
    F_CPU = 'F_CPU'
    F_GPU = 'F_GPU'
    GPU = 'GPU'
    CPU_N_CORES = 'N_THREADS'
    THR = 'THROUGHPUT'
    PRECISION = 'PRECISION'
    KERNEL_FRACTION = 'KERNEL_FRACTION'
    GPU_W = 'GPU_W'

    ACTIVE_FRACTION = 'ACTIVE_FRACTION'
#   TOTAL_W = 'TOTAL_W'

    A = '_x'
    B = '_y'
    THR_INTERFERENCE = 'THR_INTERFERENCE'
    GPU_W_INTERFERENCE = 'GPU_W_INTERFERENCE'

knowledge_dfs : Dict[str,Dict[int,Dict[str, pd.DataFrame]]] = {}

def viable_cpu(name : str, in_size : int, min_thr : float, min_precision : int = 0, throughput_coefficient : float = 1) -> pd.DataFrame :

    df = knowledge_dfs[name][in_size]["CPU"]
    if min_precision == 0 :
        viable : pd.DataFrame = df[(df[Column.THR] * throughput_coefficient >= min_thr)]
    else :
        viable : pd.DataFrame = df[(df[Column.THR] * throughput_coefficient >= min_thr) & (df[Column.PRECISION] >= min_precision)]
    viable[Column.ACTIVE_FRACTION] = min_thr / (viable[Column.THR] * throughput_coefficient)

    return viable

def viable_gpu(name : str, in_size : int, min_thr : float, min_precision :int = 0, throughput_coefficient : float = 1) -> pd.DataFrame :

    df = knowledge_dfs[name][in_size]["GPU"]
    if min_precision == 0 :
        viable : pd.DataFrame = df[(df[Column.THR] * throughput_coefficient >= min_thr)]
    else :
        viable : pd.DataFrame = df[(df[Column.THR] * throughput_coefficient >= min_thr) & (df[Column.PRECISION] >= min_precision)]
    viable[Column.ACTIVE_FRACTION] = min_thr / (viable[Column.THR] * throughput_coefficient)

    return viable

def viable_gpu_colocation(
    A_min_thr : float, A_viable : pd.DataFrame,
    B_min_thr : float, B_viable : pd.DataFrame,
    A_thr_coefficient : float = 1, B_thr_coefficient : float = 1
) -> pd.DataFrame :
        
    viable_candidates : pd.DataFrame = pd.merge(A_viable, B_viable, on=[Column.F_CPU, Column.F_GPU], how='inner')

    def thr_colocation(row) :
        A_thr_interference, B_thr_interference = prediction.gpu_thr_interference(
            row[Column.THR+Column.A],row[Column.KERNEL_FRACTION+Column.A],row[Column.ACTIVE_FRACTION+Column.A],
            row[Column.THR+Column.B],row[Column.KERNEL_FRACTION+Column.B],row[Column.ACTIVE_FRACTION+Column.B])

        gpu_w = prediction.gpu_pow_interference(
            row[Column.F_GPU],
            row[Column.THR+Column.A],A_thr_interference,row[Column.GPU_W+Column.A],
            row[Column.THR+Column.A],B_thr_interference,row[Column.GPU_W+Column.A])
        
        return (A_thr_interference, B_thr_interference, gpu_w)
    
    viable_candidates[[Column.THR_INTERFERENCE+Column.A,Column.THR_INTERFERENCE+Column.B,Column.GPU_W_INTERFERENCE]] = viable_candidates.apply(thr_colocation, axis=1, result_type='expand')
    viable_candidates = viable_candidates[
        (viable_candidates[Column.THR_INTERFERENCE+Column.A] * A_thr_coefficient >= A_min_thr) &
        (viable_candidates[Column.THR_INTERFERENCE+Column.B] * B_thr_coefficient >= B_min_thr)
    ]
    return viable_candidates
