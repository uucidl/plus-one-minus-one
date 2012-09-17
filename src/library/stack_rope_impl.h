/* a10 508
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stack_rope_impl.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 508 */



static inline
int stack_rope_is_full(stack_rope_t* self)
{
  return self->next_n == STACK_ROPE_ELEM_N;
}

static inline
int stack_rope_is_empty(stack_rope_t* self)
{
  return self->next_n == 0;
}

static inline
void stack_rope_push(stack_rope_t* self, void* obj)
{
  if(self->next_n < STACK_ROPE_ELEM_N) {
    self->elems[self->next_n++] = obj;
  }
}

static inline
void* stack_rope_pop(stack_rope_t* self)
{
  void* ret = NULL;
  if(self->next_n > 0) {
    ret = self->elems[--self->next_n];
  }

  return ret;
}

static inline
void* stack_rope_top(stack_rope_t* self)
{
  void* ret = NULL;
  if(self->next_n > 0) {
    ret = self->elems[self->next_n - 1];
  }
  
  return ret;
}
