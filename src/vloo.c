/* a10 946
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vloo.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 946 */

#include <libc/math.h>
#include <libc/stdlib.h>

#include "vloo.h"

#include <lib/chance.h>
#include <lib/pixel.h>

#include "velocity.h"
#include <audio/wavetable.h>
#include <audio/wrapper.h>

/* configuration parameters for the particle allocation */
#define MAX_PARTS_PER_SING 96
#define MAX_SINGS 12

typedef struct vloo_particle_t {
    particle_t super;
    void (*set)(struct vloo_particle_t *self, float u, float v, float w,
                float s, double birth_ms);
    void (*update)(struct vloo_particle_t *self,
                   velocity_t *instant_vel); /* update pos / instant vel */

    double birth_ms;
    velocity_t vel;
    int alive_p; /* alive flag */
} vloo_particle_t;

CLASS_INHERIT(vloo_particle, particle)

static void set_p(vloo_particle_t *self, float u, float v, float w, float s,
                  double birth_ms)
{
    self->super.x = u;
    self->super.y = v;
    self->super.z = w;
    self->super.amp = s;
    self->birth_ms = birth_ms;

    self->alive_p = 1;
}

static void update_p_v(vloo_particle_t *self, velocity_t *instant_vel)
{
    self->vel.update(&self->vel, instant_vel);
}

vloo_particle_t *vloo_particle_instantiate(vloo_particle_t *x)
{
    vloo_particle_t *v = vloo_particle_instantiate_super(x);

    v->set = set_p;
    v->update = update_p_v;

    return v;
}

enum singularity_type_enum { REPULSIVE, ROTATIVE };

typedef struct vloo_singularity_t {
    object_t super;
    int (*new)(struct vloo_singularity_t *self, float x, float y, float z,
               double strength, float rate, double birthdate);
    void (*generate)(struct vloo_singularity_t *self, double time);

    void (*update)(struct vloo_singularity_t *self, double ms);

    velocity_t *(*get_velocity)(struct vloo_singularity_t *self, float x,
                                float y, float z);

    float x, y, z;   /* position                           */
    double strength; /* current strength                   */
    double initial_strength;
    float rate; /* particle generation period         */
    double birth_ms;
    enum singularity_type_enum type;

    /* high level particles spawned by the sing. */
    vloo_particle_t *particles;
    unsigned int particle_number; /* number of next particle created */
} vloo_singularity_t;

CLASS_INHERIT(vloo_singularity, object)

static int new_s(vloo_singularity_t *self, float x, float y, float z,
                 double strength, float rate, double birthdate)
{
    self->x = x;
    self->y = y;
    self->z = z;
    self->strength = strength;
    self->initial_strength = strength;
    self->rate = rate;
    self->birth_ms = birthdate;
    self->type = REPULSIVE; // by default

    self->particles = calloc(MAX_PARTS_PER_SING, sizeof(vloo_particle_t));
    {
        int i;
        for (i = 0; i < MAX_PARTS_PER_SING; i++) {
            vloo_particle_instantiate_toplevel(self->particles + i);
        }
    }

    self->particle_number = 0;

    self->generate(self, birthdate);

    return 1;
}

#ifndef M_PI
#define M_PI 3.141592
#endif

static void v_s_generate(vloo_singularity_t *self, double time)
{
    double radius = 0.06;

// FIXME: generation rate to be implemented
#if 0
    double angle = 2*M_PI / (1.0 * MAX_PARTS_PER_SING);
    double curangle = 0;

    { int d; for(d = 0; d < MAX_PARTS_PER_SING; d++) {
      double rr = variatef(radius, radius*3.0/*.65*/);
	vloo_particle_t* p = self->particles+d;
	double cosval = rr * cos(curangle);
	double sinval = rr * sin(curangle);
	curangle+=angle;
	
	p->set(p, self->x + cosval, self->y + sinval, self->z, 1.0f, time);
    } }
#else
    {
        int d;
        for (d = 0; d < MAX_PARTS_PER_SING; d++) {
            vloo_particle_t *p = self->particles + d;
            double u, v;
            double rr = variatef(3.0 * radius, 2.0 * radius);

            unirand2d(&u, &v);
            u *= rr;
            v *= rr;
            p->set(p, self->x + u, self->y + v, variate(self->z, 0.15), 1.0f,
                   time);
        }
    }
#endif

    self->particle_number = 0;
}

static void v_s_update(vloo_singularity_t *self, double ms)
{
    // self->strength *= 0.999991;
    self->strength *= 0.99991;
    if (self->strength <= self->initial_strength * 0.01) {
        self->strength = self->initial_strength;
    }
}

static velocity_t *v_s_get_velocity(vloo_singularity_t *self, float x, float y,
                                    float z)
{
    static velocity_t vel;
    float xr = x - self->x;
    float yr = y - self->y;
    float zr = z - self->z;
    float r = xr * xr + yr * yr + zr * zr;

    if (r <= 0.00001) {
        vel.inf_p = 1;
        return &vel;
    } else
        vel.inf_p = 0;

    {
        const float s = self->strength / r;
        if (self->type == REPULSIVE) {
            vel.u = s * xr;
            vel.v = s * yr;
            vel.w = s * zr;
        } else /*if(type == SING_REPULSIVE)*/ {
            vel.u = s * yr;
            vel.v = s * xr;
            vel.w = s * zr;
        }
    }

    return &vel;
}

vloo_singularity_t *vloo_singularity_instantiate(vloo_singularity_t *x)
{
    vloo_singularity_t *v = vloo_singularity_instantiate_super(x);

    v->new = new_s;
    v->generate = v_s_generate;
    v->update = v_s_update;
    v->get_velocity = v_s_get_velocity;
    v->birth_ms = HUGE_VAL; /* far in the future == dead */

    return v;
}

/* vloo_t */

static float flare_gen(unsigned int d2, unsigned int of)
{
    float val = 1.0f;
    float d = sqrt(d2 / 4);

    if (d >= .01) {
        val = (0.8f + 0.2f * sin(d2 / 256)) / d;
    }

    return val;
}

static int vloo_new(effect_t *self)
{
    int ret = 1;
    vloo_t *vloo = (vloo_t *)self;

    ret = ret &&
          vloo->ps->new (vloo->ps, MAX_SINGS * MAX_PARTS_PER_SING, 32,
                         flare_gen, 226, 212, 195);

    {
        vloo_singularity_t *t;

        t = vloo->new_singularity(vloo, 0.0);
        t->strength = .0001;
        t = vloo->new_singularity(vloo, 300.0);
        t->strength = .0001;
        t = vloo->new_singularity(vloo, 600.0);
        t->strength = .0001;
        t = vloo->new_singularity(vloo, 900.0);
        t->strength = .0001;
        t = vloo->new_singularity(vloo, 1200.0);
        t->strength = .0001;
        t = vloo->new_singularity(vloo, 1500.0);
        t->strength = .0001;
        t = vloo->new_singularity(vloo, 1800.0);
        t->strength = .0001;
    }

    {
        vloo_singularity_t *s = vloo->new_singularity(vloo, 2000.0);
        s->type = ROTATIVE;
        s->y += .3f;
    }

    {
        vloo_singularity_t *s = vloo->new_singularity(vloo, 8000.0);
        s->type = ROTATIVE;
        s->x -= .2f;
        s->strength *= 3.2;
    }

    {
        vloo_singularity_t *s = vloo->new_singularity(vloo, 2540.0);
        s->x = .7f;
        s->y = .1f;
        s->type = ROTATIVE;
        s->strength *= 5.1;
    }

    {
        vloo_singularity_t *s = vloo->new_singularity(vloo, 5540.0);
        s->x = .8f;
        s->y = .9f;
        s->strength *= 2.1;
    }

    return ret;
}

static int vloo_destroy(effect_t *self)
{
    int ret = 1;
    vloo_t *vloo = (vloo_t *)self;

    if (vloo->ps->destroy(vloo->ps))
        object_retire(ps_to_object(vloo->ps));
    else
        ret = 0;

    return ret;
}

static vloo_singularity_t *vloo_add_singularity(vloo_t *vloo, double now)
{
    int index = vloo->singularity_number % MAX_SINGS;
    vloo_singularity_t *sing = vloo->singularities + index;

    sing->new (sing, 0.5f, 0.5f, 0.56f, 0.0002, 0.010f, now);
    vloo->singularity_number = (index + 1) % MAX_SINGS;

    return sing;
}

static void vloo_computes(effect_t *self, void *content, double ms)
{
    vloo_t *vloo = (vloo_t *)self;
    int32_t *__restrict__ pixels = content;

    int ps_i = 0; // index in rendering particle system

    /* for each singularity in the system */
    {
        int is;
        for (is = 0; is < vloo->singularity_number; is++) {
            vloo_singularity_t *sing = vloo->singularities + is;

            if (sing->birth_ms > ms)
                continue;

            /* for each particle in the singularity */
            {
                int ip;
                for (ip = 0; ip < MAX_PARTS_PER_SING; ip++) {
                    vloo_particle_t *vp = sing->particles + ip;
                    particle_t *p = (particle_t *)vp;
                    float f;

                    sing->update(sing, ms); // age

                    if (!vp->alive_p || vp->birth_ms > ms || p->x < 0.0f ||
                        p->x > 1.0f || p->y < 0.0f || p->y > 1.0f ||
                        p->z < 0.3f || p->z > 1.5f) {
                        vp->alive_p = 0;
                        continue;
                    }

                    if (ms - vp->birth_ms >= 9000) {
                        sing->generate(sing, ms);
                        break;
                    }

                    f = 0.40f * (1.0f - (ms - vp->birth_ms) / 9000.0f);
                    if (f <= 0.001f) {
                        vp->alive_p = 0;
                        break;
                    }

                    vloo->ps->particles[ps_i].amp = f;

                    vloo->ps->particles[ps_i].x =
                        (p->x - 0.5f) / p->z * vloo->super.width / 2 +
                        vloo->super.width / 2;
                    vloo->ps->particles[ps_i].y =
                        (p->y - 0.5f) / p->z * vloo->super.height / 2 +
                        vloo->super.height / 2;
                    vloo->ps->particles[ps_i].size = 5.0 / p->z;
                    vloo->ps->particles[ps_i].size_phase =
                        vloo->ps->opacity_r / vloo->ps->particles[ps_i].size;
                    ps_i++;

                    /* then update position */
                    {
                        float u = 0.0f;
                        float v = 0.0f;
                        float w = 0.0f;
                        int inf_p = 0;
                        velocity_t *vel;
                        int iis;

                        for (iis = 0; iis < vloo->singularity_number; iis++) {
                            vloo_singularity_t *s = vloo->singularities + iis;

                            if (s->birth_ms > ms)
                                continue;
                            vel = s->get_velocity(s, p->x, p->y, p->z);
                            if (vel->inf_p) {
                                inf_p = 1;
                                break;
                            }

                            u += vel->u;
                            v += vel->v;
                            w += vel->w;
                        }
                        if (inf_p) {
                            vp->alive_p = 0;
                            continue;
                        }

                        p->x += u;
                        p->x = variatef(p->x, 0.003f);
                        p->y += v;
                        p->y = variatef(p->y, 0.003f);
                        p->z += w;
                        p->z = variatef(p->z, 0.0003f);
                    }
                }
            }
        }
    }

    dim2(pixels, vloo->super.width * vloo->super.height);
    vloo->ps->render(vloo->ps, pixels, vloo->super.width, vloo->super.height,
                     vloo->super.pitch);
}

vloo_t *vloo_instantiate(vloo_t *x)
{
    vloo_t *v = vloo_instantiate_super(x);

    ((effect_t *)v)->new = vloo_new;
    ((effect_t *)v)->destroy = vloo_destroy;
    ((effect_t *)v)->computes = vloo_computes;

    v->new_singularity = vloo_add_singularity;

    v->ps = ps_instantiate_toplevel(NULL);
    v->singularities = calloc(sizeof(vloo_singularity_t), MAX_SINGS);
    {
        int i;
        for (i = 0; i < MAX_SINGS; i++) {
            vloo_singularity_instantiate_toplevel(v->singularities + i);
        }
    }
    v->singularity_number = 0;

    return v;
}
