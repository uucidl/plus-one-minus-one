/* a10 1
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/object_macros.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 1 */




#ifndef KNOS_LIBRARY_OBJECT_MACROS_H
#define KNOS_LIBRARY_OBJECT_MACROS_H

#ifndef NDEBUG

#define OBJECT_INSTANTIATE(class, ancestor) \
    (class##_t*) object_allocate_debug(class##_to_object(ancestor), sizeof(class##_t), #class, __FILE__, __LINE__) 
    
#else

#define OBJECT_INSTANTIATE(class, ancestor) \
    (class##_t*) object_allocate(class##_to_object(ancestor), sizeof(class##_t))

#endif

#define OBJECT_TOPLEVEL(object) \
  do { \
    object_t* o = (object_t*) (object); \
    o->magic = 0x0; \
  } while (0);

#define OBJECT_MACROS__VISIBILITY_public extern
#define OBJECT_MACROS__VISIBILITY_private static

// visibility: public/private
#define DEFINE_OBJECT_INSTANTIATE2(class, visibility)	\
  /* declare the instantiate function */ \
  OBJECT_MACROS__VISIBILITY_##visibility class##_t* class##_instantiate (class##_t* x);	\
  /* declare a toplevel instantiator */ \
  static inline class##_t* class##_instantiate_toplevel (class##_t* x) { \
    if (x) { \
      OBJECT_TOPLEVEL (class##_to_object (x)); \
    } \
    return class##_instantiate (x); \
  }

#define DEFINE_OBJECT_INSTANTIATE(class) DEFINE_OBJECT_INSTANTIATE2(class, public)

#define CLASS_INHERIT2(visibility, class, parent_class)	\
  /* defines a 'caster' function */ \
  static inline object_t* class##_to_object(class##_t* x) { \
    return parent_class##_to_object(&x->super); \
  } \
  static inline parent_class##_t* class##_to_superclass (class##_t* x) { \
    return &x->super; \
  } \
  /* defines a 'retire' function */ \
  static inline void class##_retire(class##_t* x) { \
    object_retire(class##_to_object(x)); \
  } \
  /* define a class preliminary instantiator */ \
  static inline class##_t* class##_instantiate_super(class##_t* x) { \
     class##_t* object = OBJECT_INSTANTIATE (class, x); \
     parent_class##_instantiate (&object->super); \
     return object; \
  } \
  DEFINE_OBJECT_INSTANTIATE2(class, visibility)
  
#define CLASS_INHERIT(class, parent_class) \
	CLASS_INHERIT2(public, class, parent_class)

#endif
