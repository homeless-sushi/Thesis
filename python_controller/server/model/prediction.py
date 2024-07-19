from typing import Tuple

import model.system as system

def cpu_pow(cpu_frq : int, u1 : float = 0, u2 : float = 0, u3 : float = 0, u4 : float = 0) -> float:

    idle = system.cpu_idle_power[int(cpu_frq)]
    u100, u200, u300, u400 = system.cpu_power[int(cpu_frq)] 
    total_power = idle + u100*u1 + u200*u2 + u300*u3 + u400*u4

    return total_power

def gpu_thr_interference(
    A_thr : float, A_kernel_fraction : float, A_active_fraction : float,
    B_thr : float, B_kernel_fraction : float, B_active_fraction : float
) -> Tuple[float, float]:

    A_gpu_fraction = A_kernel_fraction * A_active_fraction
    A_cpu_fraction = 1 - A_gpu_fraction

    B_gpu_fraction = B_kernel_fraction * B_active_fraction
    B_cpu_fraction = 1 - B_gpu_fraction

    A_t_tot = 1/A_thr
    A_t_gpu = A_t_tot*A_gpu_fraction
    A_t_cpu = A_t_tot-A_t_gpu

    B_t_tot = 1/B_thr
    B_t_gpu = B_t_tot*B_gpu_fraction
    B_t_cpu = B_t_tot-B_t_gpu

    A_t_wait = max(B_t_gpu-A_t_cpu, 0)
    B_t_wait = max(A_t_gpu-B_t_cpu, 0)

    A_t_tot_interference = B_cpu_fraction*A_t_tot + B_gpu_fraction*(A_t_tot + A_t_wait)
    B_t_tot_interference = A_cpu_fraction*B_t_tot + A_gpu_fraction*(B_t_tot + B_t_wait)

    A_thr_interference = 1/A_t_tot_interference
    B_thr_interference = 1/B_t_tot_interference

    return (A_thr_interference, B_thr_interference)

def gpu_pow_interference(
    gpu_frq : int, 
    A_thr : float, A_thr_interference : float, A_gpu_w : float, 
    B_thr : float, B_thr_interference : float, B_gpu_w : float
) -> float:

    base_power = system.gpu_idle_power[int(gpu_frq)]

    total_power = (
        (A_gpu_w - base_power)*(A_thr_interference/A_thr) +
        (B_gpu_w - base_power)*(B_thr_interference/B_thr) +
        base_power
    )

    return total_power

def cpu_app_power(cpu_frq : int, kernel_fraction : float, active_fraction : float, n_cpu_cores : int) -> float:

    utilizations = (
        [active_fraction] 
        + ([kernel_fraction*active_fraction] * (n_cpu_cores-1)) 
    )
    return cpu_pow(cpu_frq, *utilizations)

def gpu_app_power(cpu_frq : int, kernel_fraction : float, active_fraction : float, gpu_w : float) -> float:

    cpu_fraction = (1-kernel_fraction)*active_fraction
    cpu_contribution = cpu_pow(cpu_frq, cpu_fraction)
    gpu_contribution = gpu_w*active_fraction
    return cpu_contribution+gpu_contribution

def gpu_apps_power(
    cpu_frq : int, 
    A_kernel_fraction : float, A_active_fraction : float,
    B_kernel_fraction : float, B_active_fraction : float, 
    total_gpu_w : float
) -> float:

    A_cpu_fraction = (1-A_kernel_fraction)*A_active_fraction
    B_cpu_fraction = (1-B_kernel_fraction)*B_active_fraction
    return cpu_pow(cpu_frq, A_cpu_fraction, B_cpu_fraction) + total_gpu_w

