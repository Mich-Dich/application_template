#pragma once


class GUID {
public:

	// Declares the default copy constructor for GUID.
	// @param other The GUID instance to copy from.
	// @return None.
	DEFAULT_COPY_CONSTRUCTOR(GUID);
	
	// Constructs a new GUID by generating a random 64-bit value using the
	// library's global RNG and distribution.
	// @return None. (A new GUID object is constructed with a generated value.)
	GUID();

	// Constructs a GUID from an existing 64-bit value.
	// @param uuid The explicit 64-bit value to initialize this GUID with.
	// @return None. (A new GUID object is constructed with [uuid].)
	GUID(u64 uuid);

	// Destroys this GUID. If experimental collision avoidance is enabled,
	// the destructor will remove this GUID's value from the set of generated GUIDs.
	// @return None.
	~GUID();

	// Conversion operator that returns the underlying 64-bit GUID value.
	// @return The 64-bit integer representation of this GUID.
	operator u64() const { return m_GUID; }

private:

	u64 m_GUID;
};


namespace std {

	template <typename T> struct hash;

	// Specialization of std::hash for GUID so GUIDs can be used as keys in
	// unordered containers (e.g. std::unordered_map, std::unordered_set).
	template<>
	struct hash<GUID> {

		// Produces a hash value for a GUID by returning its underlying 64-bit value
		// converted to std::size_t.
		// @param uuid The GUID to hash.
		// @return A std::size_t hash derived from the GUID's 64-bit value.
		std::size_t operator()(const GUID& uuid) const { return (u64)uuid; }
	};

}
