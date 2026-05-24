/* File: oh_particle_ownership.h
   Particle-buffer ownership flags for v2 context binding APIs.
*/
#ifndef OH_PARTICLE_OWNERSHIP_H
#define OH_PARTICLE_OWNERSHIP_H

/* OhHelp borrows application-owned storage. */
#define OH_PARTICLES_BORROWED 0

/* OhHelp allocates and owns storage for the current context. */
#define OH_PARTICLES_OWNED 1

#endif
