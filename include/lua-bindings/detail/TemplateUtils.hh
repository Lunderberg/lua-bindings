#ifndef _TEMPLATEUTILS_H_
#define _TEMPLATEUTILS_H_

// Template metaprogramming, described in http://loungecpp.wikidot.com/tips-and-tricks%3aindices
// Creates a compile-time listing of indices, useful for passing index parameters.
// To use, declare a function as follows:
//   template<int... Is>
//   void func(indices<Is...>);
// Then call the function as follows:
//   func(build_indices<5>());
// This will define the index template variables.
template<int... Is>
struct indices {};

template<int N, int... Is>
struct build_indices
  : build_indices<N-1, N-1, Is...> {};

template<int... Is>
struct build_indices<0, Is...> : indices<Is...> {};

#endif /* _TEMPLATEUTILS_H_ */
