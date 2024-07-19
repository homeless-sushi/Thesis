from collections import deque
from typing import Deque

power_limit : int = 2600
power_tolerance : float = 0.1
power_rolling_avg_mem : int = 5 
power_rolling_avg : Deque[float] = deque(
    [1]*power_rolling_avg_mem,
    maxlen=power_rolling_avg_mem
)

def update_coefficient(prescribed : float, measured : float) -> None :

    if prescribed == 0:
        return
    
    power_rolling_avg.append(measured/prescribed)

def reset_coefficient() -> None :
    power_rolling_avg.clear()
    power_rolling_avg.extend([1] * power_rolling_avg_mem)

def check_violation(power : float) -> bool :
    avg : float = sum(power_rolling_avg) / len(power_rolling_avg)
    return (avg * power) > (power_limit * (1+power_tolerance))
