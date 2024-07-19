class Location:
    CPU = 'CPU'
    GPU = 'GPU'
    NONE = 'NONE'
    UNVIABLE = 'UNVIABLE'

class App:

    def __init__(self,
        pid : int, name : str, size : int, max_cpu_cores : int, is_gpu: bool, is_approximate : bool, 
        min_thr: float, min_precision: int,
        curr_thr: float, curr_precision: int, location: Location, cpu_cores: int
    ) :
    
        self.pid : int = pid
        self.name : str = name
        self.size : int = size
        self.max_cpu_cores : int = max_cpu_cores
        self.is_gpu : bool = is_gpu
        self.is_approximate : bool = is_approximate

        self.min_thr : float = min_thr
        self.min_precision : int = min_precision
        
        self.curr_thr : float = curr_thr
        self.curr_precision : int = curr_precision
        self.location : Location = location
        self.cpu_cores : int = cpu_cores