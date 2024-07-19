from . import state
from . import idle
from . import decide
from . import refine

import policy.utility.system_state_utility as system_state_utility
import policy.utility.app_throughput as app_throughput
import policy.utility.system_power as system_power

from model.system_state import SystemState

class WaitRefine(state.State) :
    pass

class WaitRefine(state.State) :
    WAIT_FOR : int = 2

    def __init__(self, prev_prescribed: SystemState, curr_measured: SystemState, wait_counter : int = None):
        super().__init__(prev_prescribed, curr_measured)
        
        if wait_counter is None :
            self.wait_counter = WaitRefine.WAIT_FOR
        else :
            self.wait_counter = wait_counter

    def next(self, new_measured : SystemState) -> state.State :

        # Applications enter or exit the system
        new_pids, deleted_pids = (
            system_state_utility.compare_system_states(self.curr_prescribed, new_measured)
        )
        if len(new_pids) > 0 or len(deleted_pids) > 0 :
            return decide.Decide(self.curr_prescribed, new_measured, reset_power=True)
        
        # Update power coefficient
        system_power.update_coefficient(self.curr_prescribed.power, new_measured.power)
        # Update throughput coeffiecients
        app_throughput.update_coefficients(self.curr_prescribed, new_measured)
         
        # Waiting over
        if self.wait_counter == 0 :

            # if Power violations
            if system_power.check_violation(new_measured.power) :
                return refine.Refine(self.curr_prescribed, new_measured)
            
            else :
                return idle.Idle(self.curr_prescribed, new_measured)
        
        # Wait more        
        else :
            return WaitRefine(self.curr_prescribed, new_measured, self.wait_counter-1)
    
    def handle(self) :
        self.curr_prescribed = self.prev_prescribed