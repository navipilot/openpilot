"""Compatibility alias for legacy imports.

Prefer: openpilot.selfdrive.pandad.pandad_api_impl
"""

from openpilot.selfdrive.pandad.pandad_api_impl import can_capnp_to_list, can_list_to_can_capnp

__all__ = ["can_list_to_can_capnp", "can_capnp_to_list"]
