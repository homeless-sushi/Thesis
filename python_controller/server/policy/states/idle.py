from . import state
from . import decide
from . import refine

import policy.utility.system_state_utility as system_state_utility
import policy.utility.system_power as system_power
import policy.utility.app_throughput as app_throughput

from model.system_state import SystemState

class Idle(state.State) :

    def __init__(self, prev_prescribed: SystemState, curr_measured: SystemState):
        super().__init__(prev_prescribed, curr_measured)

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

        # Power violations
        if system_power.check_violation(new_measured.power) :

            # If precision is minimum
            if all(app.curr_precision == app.min_precision  for app in new_measured.current_apps.values()) :
                return decide.Decide(self.curr_prescribed, new_measured)
            else :
                return refine.Refine(self.curr_prescribed, new_measured)
            
        # Throughput violations
        if app_throughput.check_violation(new_measured) :
            return decide.Decide(self.curr_prescribed, new_measured)
 
        # Nothing has happened
        else :
            return Idle(self.curr_prescribed, new_measured)
    
    def handle(self) :
        self.curr_prescribed = self.curr_measured