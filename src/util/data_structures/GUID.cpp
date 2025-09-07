
#include "util/pch.h"

#include "GUID.h"


//#define USE_EXPERIMENTAL_COLLISION_AVOIDANCE

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<u64> s_UniformDistribution;

#ifdef USE_EXPERIMENTAL_COLLISION_AVOIDANCE
	static std::unordered_set<u64> s_generated_GUIDs;		// To track generated GUIDs and avoid duplicates
	FORCEINLINE static bool is_unique(u64 uuid) { return s_generated_GUIDs.find(uuid) == s_generated_GUIDs.end(); }
#endif // USE_EXPERIMENTAL_COLLISION_AVOIDANCE

	GUID::GUID() {
#ifdef USE_EXPERIMENTAL_COLLISION_AVOIDANCE

		do {
			m_GUID = s_UniformDistribution(s_Engine);
		} while (!is_unique(m_GUID));
		s_generated_GUIDs.insert(m_GUID);
#else
		m_GUID = s_UniformDistribution(s_Engine);
#endif // USE_EXPERIMENTAL_COLLISION_AVOIDANCE
	}

	GUID::GUID(u64 uuid)
		: m_GUID(uuid) { }

	GUID::~GUID() { 
#ifdef USE_EXPERIMENTAL_COLLISION_AVOIDANCE
		s_generated_GUIDs.erase(m_GUID);
#endif // USE_EXPERIMENTAL_COLLISION_AVOIDANCE
	}

