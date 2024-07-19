from typing import Optional

from model.system_state import SystemState

class State :
    pass

class State :
    """A class representing a policy's State.

    Attributes:
        prev_prescribed : SystemState
            the previously system_state prescribed by the policy
        curr_measured : SystemState
            the current measured system_state
        curr_prescribed : Optional[SystemState]
            the system_state prescribed by this states, set by using handle()
    """

    def __init__(self, prev_prescribed : SystemState, curr_measured: SystemState):
        self.prev_prescribed : SystemState = prev_prescribed
        self.curr_measured : SystemState = curr_measured
        self.curr_prescribed : Optional[SystemState] =  None

    def next(self, new_measured : SystemState) -> State :
        """Returns the next policy state, given the new measured system state"""
        raise NotImplementedError

    def handle(self) -> None :
        """Handles the curr_state (measured system state), and sets the
        curr_prescribed state
        """
        raise NotImplementedError
    
