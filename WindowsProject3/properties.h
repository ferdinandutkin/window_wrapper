#include <functional>
#include <iostream>

template<class T>
using get_func = std::function<T()>;

template<class T>
using set_func = std::function<void(const T&)>;

template<class T>
class prop {
	get_func<T> get;
	set_func<T> set;
public:
	prop(get_func<T> getter, set_func<T> setter) : get(getter), set(setter)
	{ }

	prop(T& to_get, set_func<T> setter) : get([&] {return to_get; }), set(setter)
	{ }

	friend std::ostream& operator << (std::ostream& os, const prop p) {
		os << p.get();
		return os;
	}
	operator T () const { return get(); }
	void operator = (const T& t) { set(t); }
};

