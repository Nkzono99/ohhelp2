#ifndef OH_PARTICLE_STORAGE_INTERNAL_H
#define OH_PARTICLE_STORAGE_INTERNAL_H

#include "oh_particle_storage.h"
#include "oh_particle_adapter.h"

struct oh_soa_bank;
struct oh_soa_storage {
  int level4;
  struct oh_soa_bank *banks[3];
  MPI_Datatype cursor_type;
  /* Level 4's private adapter must not replace the application's AoS setup. */
  oh_particle_adapter saved_aos_adapter;
  MPI_Datatype saved_aos_type;
  int saved_aos_kind;
  MPI_Comm comm;
  int nfields;
  oh_particle_field *fields;
  size_t *offsets;
  void **bases;
  int region_field;
  int species_field;
  int species_base;
  size_t record_size;
  int capacity;
  int ownership;
  MPI_Datatype wire_type;
  void *receive_buffer;
  void **receive_starts;
};

void oh_soa_destroy(struct oh_soa_storage *storage);
void oh_soa_unbind(struct oh_soa_storage *storage);
int oh_soa_bind(struct oh_soa_storage *storage, void *const *bases,
                int capacity, int ownership);
void oh_storage_require_aos(struct oh_state *state, const char *api);
void oh_storage_advance_generation(struct oh_state *state);
void oh_storage_preflight(struct oh_state *state, int level);
void oh_soa_check_capacity(struct oh_state *state, long long count);
long long oh_soa_region(struct oh_soa_storage *storage, int index);
int oh_soa_species(struct oh_state *state, int index);
void oh_soa_set_region(struct oh_soa_storage *storage, int index,
                       long long region);
void oh_soa_copy(struct oh_soa_storage *storage, int dst, int src, int count);
void oh_soa_pack(struct oh_soa_storage *storage, void *records,
                 int src, int count);
void oh_soa_unpack(struct oh_soa_storage *storage, int dst,
                   const void *records, int count);
void oh_soa_gather(struct oh_soa_storage *storage, const int *indices, int count);
void *oh_storage_receive_at(struct oh_state *state, int index);
int oh_storage_receive_index(struct oh_state *state, void *records, int count);
void oh_storage_unpack_received(struct oh_state *state, void *records, int count);

/* Level 4's legacy cursors refer to metadata slots (bank + index), never to
   particle records. Payloads stay in native columns in all three banks. */
int oh_soa_is_level4(const struct oh_state *state);
void oh_soa_level4_init(struct oh_state *state, int kind);
void *oh_soa_level4_bank(struct oh_state *state, int bank, int capacity);
void oh_soa_level4_finish(struct oh_state *state);
void oh_soa_level4_release(struct oh_soa_storage *s);
void oh_soa_level4_restore_adapter(struct oh_state *state);
void oh_soa_cursor_copy(struct oh_state *state, void *dst, const void *src, int count);
double *oh_soa_cursor_position(struct oh_state *state, void *cursor, int dim);
void *oh_soa_level4_slot(struct oh_state *state, int index);
int oh_soa_append(struct oh_state *state, const void *const *values, int nfields,
                  oh_particle_ref *ref);
double *oh_soa_position(struct oh_state *state, int index, int dim);
int oh_storage_isend(struct oh_state *state, const void *buf, int count,
                     MPI_Datatype type, int peer, int tag, MPI_Comm comm, MPI_Request *req);
int oh_storage_irecv(struct oh_state *state, void *buf, int count,
                     MPI_Datatype type, int peer, int tag, MPI_Comm comm, MPI_Request *req);
int oh_storage_send(struct oh_state *state, const void *buf, int count,
                    MPI_Datatype type, int peer, int tag, MPI_Comm comm);
int oh_storage_recv(struct oh_state *state, void *buf, int count,
                    MPI_Datatype type, int peer, int tag, MPI_Comm comm, MPI_Status *status);
int oh_storage_sendrecv(struct oh_state *state, const void *sbuf, int scount,
                        MPI_Datatype stype, int dst, int stag, void *rbuf, int rcount,
                        MPI_Datatype rtype, int src, int rtag, MPI_Comm comm, MPI_Status *status);
int oh_storage_alltoallv(struct oh_state *state, const void *sbuf, const int *scount,
                         const int *sdisp, MPI_Datatype stype, void *rbuf,
                         const int *rcount, const int *rdisp, MPI_Datatype rtype, MPI_Comm comm);
#endif
