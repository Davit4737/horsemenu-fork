include(FetchContent)

message(STATUS "RDR-Classes")

FetchContent_Declare(
    RDR-Classes
    GIT_REPOSITORY https://github.com/YimMenu/RDR-Classes.git
    GIT_TAG        a61459d3b100408f736f32046ed2545dc729e617
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(RDR-Classes)

FetchContent_GetProperties(RDR-Classes SOURCE_DIR _RDR_CLASSES_SOURCE_DIR)

# MSVC /std:c++latest rejects "return T*(this);" inside templates (C3878). Patch after fetch.
set(_RLSC_SESSION_EVENT_HPP "${_RDR_CLASSES_SOURCE_DIR}/network/rlScSessionEvent.hpp")
if(EXISTS "${_RLSC_SESSION_EVENT_HPP}")
	file(READ "${_RLSC_SESSION_EVENT_HPP}" _RLSC_SESSION_EVENT_CONTENT)
	if(_RLSC_SESSION_EVENT_CONTENT MATCHES "return T\\*\\(this\\);")
		string(REPLACE "return T*(this);" "return reinterpret_cast<T*>(this);" _RLSC_SESSION_EVENT_CONTENT "${_RLSC_SESSION_EVENT_CONTENT}")
		file(WRITE "${_RLSC_SESSION_EVENT_HPP}" "${_RLSC_SESSION_EVENT_CONTENT}")
		message(STATUS "Patched rlScSessionEvent.hpp for MSVC template cast parsing")
	endif()
endif()

set_property(TARGET RDR-Classes PROPERTY CXX_STANDARD 23)