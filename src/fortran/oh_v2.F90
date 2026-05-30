!  File: oh_v2.F90
!  Fortran ISO_C_BINDING facade for v2 context and particle adapters.
#include "oh_config.h"
module ohhelp_v2
  use iso_c_binding
  implicit none
  private

  type :: oh_context_handle
    type(c_ptr) :: ptr = c_null_ptr
  end type

  type :: oh_particle_adapter_handle
    type(c_ptr) :: ptr = c_null_ptr
  end type

  integer(c_int), parameter, public :: OH_PARTICLES_BORROWED = 0_c_int
  integer(c_int), parameter, public :: OH_PARTICLES_OWNED = 1_c_int
  integer(c_int), parameter, public :: OH_MODE_NORMAL_PRIMARY = 0_c_int
  integer(c_int), parameter, public :: OH_MODE_NORMAL_SECONDARY = 1_c_int
  integer(c_int), parameter, public :: OH_MODE_REBALANCE_SECONDARY = -1_c_int
  integer(c_int), parameter, public :: OH_MODE_ANY_PRIMARY = 2_c_int
  integer(c_int), parameter, public :: OH_MODE_ANY_SECONDARY = 3_c_int
  integer(c_int), parameter, public :: OH_MODE_NORM_PRI = &
    OH_MODE_NORMAL_PRIMARY
  integer(c_int), parameter, public :: OH_MODE_NORM_SEC = &
    OH_MODE_NORMAL_SECONDARY
  integer(c_int), parameter, public :: OH_MODE_REB_SEC = &
    OH_MODE_REBALANCE_SECONDARY
  integer(c_int), parameter, public :: OH_MODE_ANY_PRI = OH_MODE_ANY_PRIMARY
  integer(c_int), parameter, public :: OH_MODE_ANY_SEC = OH_MODE_ANY_SECONDARY

  type, bind(C) :: oh_mycomm_v2
    integer(c_int) :: prime
    integer(c_int) :: sec
    integer(c_int) :: rank
    integer(c_int) :: root
    integer(c_int) :: black
  end type

  abstract interface
    function oh_particle_get_region_callback(adapter, particle, &
                                             primary_or_secondary) &
                                             bind(C) result(region)
      import :: c_ptr, c_int, c_long_long
      type(c_ptr), value :: adapter
      type(c_ptr), value :: particle
      integer(c_int), value :: primary_or_secondary
      integer(c_long_long) :: region
    end function

    subroutine oh_particle_set_region_callback(adapter, particle, region, &
                                               primary_or_secondary) bind(C)
      import :: c_ptr, c_int, c_long_long
      type(c_ptr), value :: adapter
      type(c_ptr), value :: particle
      integer(c_long_long), value :: region
      integer(c_int), value :: primary_or_secondary
    end subroutine

    function oh_particle_get_species_callback(adapter, particle) &
                                             bind(C) result(species)
      import :: c_ptr, c_int
      type(c_ptr), value :: adapter
      type(c_ptr), value :: particle
      integer(c_int) :: species
    end function

    function oh_particle_map_callback(adapter, particle, &
                                      primary_or_secondary) &
                                      bind(C) result(region)
      import :: c_ptr, c_int, c_long_long
      type(c_ptr), value :: adapter
      type(c_ptr), value :: particle
      integer(c_int), value :: primary_or_secondary
      integer(c_long_long) :: region
    end function
  end interface

  public :: oh_context_handle
  public :: oh_particle_adapter_handle
  public :: oh_mycomm_v2
  public :: oh_particle_get_region_callback
  public :: oh_particle_set_region_callback
  public :: oh_particle_get_species_callback
  public :: oh_particle_map_callback
  public :: oh_context_create
  public :: oh_context_destroy
  public :: oh_default_context
  public :: oh_context_configure_particles
  public :: oh_context_associated
  public :: oh_particle_adapter_associated
  public :: oh_context_set_region_weights
  public :: oh_context_set_particle_mpi_type
  public :: oh_context_set_particle_adapter
  public :: oh_context_bind_particles
  public :: oh_context_unbind_particles
  public :: oh_context_bind_region_ids
  public :: oh_context_unbind_region_ids
  public :: oh_context_get_region_ids
  public :: oh_context_bind_particle_accounting
  public :: oh_context_unbind_particle_accounting
  public :: oh_context_max_local_particles_for_capacity
  public :: oh_context_configure_level3
  public :: oh_context_configure_level3_legacy
  public :: oh_context_transbound1
  public :: oh_context_transbound2
  public :: oh_context_transbound3
  public :: oh_context_broadcast
  public :: oh_context_all_reduce
  public :: oh_context_reduce
  public :: oh_context_set_total_particles
  public :: oh_context_inject_particle
  public :: oh_context_inject_particle_get
  public :: oh_context_remap_injected_particle
  public :: oh_context_remove_injected_particle
  public :: oh_context_grid_size
  public :: oh_context_map_particle_to_neighbor
  public :: oh_context_map_particle_to_subdomain
  public :: oh_context_bcast_field
  public :: oh_context_reduce_field
  public :: oh_context_allreduce_field
  public :: oh_context_exchange_borders
  public :: oh2_init_raw
  public :: oh3_init_raw
  public :: oh_particle_adapter_create_byte
  public :: oh_particle_adapter_destroy
  public :: oh_particle_adapter_validate
  public :: oh_particle_adapter_set_mpi_type
  public :: oh_particle_adapter_set_species_base
  public :: oh_particle_adapter_use_int_fields
  public :: oh_particle_adapter_use_single_species_int_region
  public :: oh_particle_adapter_use_integer_fields
  public :: oh_particle_adapter_use_single_species_integer_region
  public :: oh_particle_adapter_use_position_fields
  public :: oh_particle_adapter_use_level3_position_fields
  public :: oh_particle_adapter_set_callbacks
  public :: oh_particle_field_offset
  public :: oh_set_particle_adapter

  interface
    function c_oh_default_context() bind(C, name="oh_fortran_default_context") &
                                  result(context)
      import :: c_ptr
      type(c_ptr) :: context
    end function

    function c_oh_context_create(fortran_comm, context) &
        bind(C, name="oh_fortran_context_create") result(ierr)
      import :: c_ptr, c_int
      integer(c_int), value :: fortran_comm
      type(c_ptr) :: context
      integer(c_int) :: ierr
    end function

    subroutine c_oh_context_destroy(context) &
        bind(C, name="oh_fortran_context_destroy")
      import :: c_ptr
      type(c_ptr), value :: context
    end subroutine

    subroutine c_oh_context_configure_particles(context, nspec, maxfrac) &
        bind(C, name="oh_fortran_context_configure_particles")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      integer(c_int), value :: nspec
      integer(c_int), value :: maxfrac
    end subroutine

    subroutine c_oh_context_set_region_weights(context, weights, weight_count) &
        bind(C, name="oh_fortran_context_set_region_weights")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: weights
      integer(c_int), value :: weight_count
    end subroutine

    subroutine c_oh_context_set_particle_mpi_type(context, fortran_type) &
        bind(C, name="oh_fortran_context_set_particle_mpi_type")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      integer(c_int), value :: fortran_type
    end subroutine

    subroutine c_oh_context_set_particle_adapter(context, adapter) &
        bind(C, name="oh_fortran_context_set_particle_adapter")
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: adapter
    end subroutine

    function c_oh_context_bind_particles(context, particles, maxlocalp, &
                                         ownership) &
        bind(C, name="oh_fortran_context_bind_particles") result(bound)
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: particles
      integer(c_int), value :: maxlocalp
      integer(c_int), value :: ownership
      type(c_ptr) :: bound
    end function

    subroutine c_oh_context_unbind_particles(context) &
        bind(C, name="oh_fortran_context_unbind_particles")
      import :: c_ptr
      type(c_ptr), value :: context
    end subroutine

    function c_oh_context_bind_region_ids(context, sdid, ownership) &
        bind(C, name="oh_fortran_context_bind_region_ids") result(bound)
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: sdid
      integer(c_int), value :: ownership
      type(c_ptr) :: bound
    end function

    subroutine c_oh_context_unbind_region_ids(context) &
        bind(C, name="oh_fortran_context_unbind_region_ids")
      import :: c_ptr
      type(c_ptr), value :: context
    end subroutine

    subroutine c_oh_context_get_region_ids(context, sdid) &
        bind(C, name="oh_fortran_context_get_region_ids")
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: sdid
    end subroutine

    subroutine c_oh_context_bind_particle_accounting(context, nphgram, &
                                                     totalp, pbase, &
                                                     ownership) &
        bind(C, name="oh_fortran_context_bind_particle_accounting")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr) :: nphgram
      type(c_ptr) :: totalp
      type(c_ptr) :: pbase
      integer(c_int), value :: ownership
    end subroutine

    subroutine c_oh_context_unbind_particle_accounting(context) &
        bind(C, name="oh_fortran_context_unbind_particle_accounting")
      import :: c_ptr
      type(c_ptr), value :: context
    end subroutine

    function c_oh_context_max_local_particles_for_capacity( &
        context, global_particle_limit, capacity_percent, min_margin) &
        bind(C, name="oh_fortran_context_max_local_particles_for_capacity") &
        result(maxlocalp)
      import :: c_ptr, c_int, c_long_long
      type(c_ptr), value :: context
      integer(c_long_long), value :: global_particle_limit
      integer(c_int), value :: capacity_percent
      integer(c_int), value :: min_margin
      integer(c_int) :: maxlocalp
    end function

    subroutine c_oh_context_configure_level3(context, pcoord, sdoms, &
        scoord, nbound, bcond, bounds, ftypes, cfields, ctypes, fsizes) &
        bind(C, name="oh_fortran_context_configure_level3")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pcoord
      type(c_ptr), value :: sdoms
      type(c_ptr), value :: scoord
      integer(c_int), value :: nbound
      type(c_ptr), value :: bcond
      type(c_ptr), value :: bounds
      type(c_ptr), value :: ftypes
      type(c_ptr), value :: cfields
      type(c_ptr), value :: ctypes
      type(c_ptr), value :: fsizes
    end subroutine

    subroutine c_oh_context_configure_level3_legacy(context, pcoord, sdoms, &
        scoord, nbound, bcond, bounds, ftypes, cfields, ctypes, fsizes) &
        bind(C, name="oh_fortran_context_configure_level3_legacy")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pcoord
      type(c_ptr), value :: sdoms
      type(c_ptr), value :: scoord
      integer(c_int), value :: nbound
      type(c_ptr), value :: bcond
      type(c_ptr), value :: bounds
      type(c_ptr), value :: ftypes
      type(c_ptr), value :: cfields
      type(c_ptr), value :: ctypes
      type(c_ptr), value :: fsizes
    end subroutine

    function c_oh_context_transbound1(context, currmode, stats) &
        bind(C, name="oh_fortran_context_transbound1") result(mode)
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      integer(c_int), value :: currmode
      integer(c_int), value :: stats
      integer(c_int) :: mode
    end function

    function c_oh_context_transbound2(context, currmode, stats) &
        bind(C, name="oh_fortran_context_transbound2") result(mode)
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      integer(c_int), value :: currmode
      integer(c_int), value :: stats
      integer(c_int) :: mode
    end function

    function c_oh_context_transbound3(context, currmode, stats) &
        bind(C, name="oh_fortran_context_transbound3") result(mode)
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      integer(c_int), value :: currmode
      integer(c_int), value :: stats
      integer(c_int) :: mode
    end function

    subroutine c_oh_context_broadcast(context, pbuf, sbuf, pcount, scount, &
                                      ptype, stype) &
        bind(C, name="oh_fortran_context_broadcast")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pbuf
      type(c_ptr), value :: sbuf
      integer(c_int), value :: pcount
      integer(c_int), value :: scount
      integer(c_int), value :: ptype
      integer(c_int), value :: stype
    end subroutine

    subroutine c_oh_context_all_reduce(context, pbuf, sbuf, pcount, scount, &
                                       ptype, stype, pop, sop) &
        bind(C, name="oh_fortran_context_all_reduce")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pbuf
      type(c_ptr), value :: sbuf
      integer(c_int), value :: pcount
      integer(c_int), value :: scount
      integer(c_int), value :: ptype
      integer(c_int), value :: stype
      integer(c_int), value :: pop
      integer(c_int), value :: sop
    end subroutine

    subroutine c_oh_context_reduce(context, pbuf, sbuf, pcount, scount, &
                                   ptype, stype, pop, sop) &
        bind(C, name="oh_fortran_context_reduce")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pbuf
      type(c_ptr), value :: sbuf
      integer(c_int), value :: pcount
      integer(c_int), value :: scount
      integer(c_int), value :: ptype
      integer(c_int), value :: stype
      integer(c_int), value :: pop
      integer(c_int), value :: sop
    end subroutine

    subroutine c_oh_context_set_total_particles(context) &
        bind(C, name="oh_fortran_context_set_total_particles")
      import :: c_ptr
      type(c_ptr), value :: context
    end subroutine

    subroutine c_oh_context_inject_particle(context, part) &
        bind(C, name="oh_fortran_context_inject_particle")
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: part
    end subroutine

    function c_oh_context_inject_particle_get(context, part) &
        bind(C, name="oh_fortran_context_inject_particle_get") result(copy)
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: part
      type(c_ptr) :: copy
    end function

    subroutine c_oh_context_remap_injected_particle(context, part) &
        bind(C, name="oh_fortran_context_remap_injected_particle")
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: part
    end subroutine

    subroutine c_oh_context_remove_injected_particle(context, part) &
        bind(C, name="oh_fortran_context_remove_injected_particle")
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: part
    end subroutine

    subroutine c_oh_context_grid_size(context, size) &
        bind(C, name="oh_fortran_context_grid_size")
      import :: c_ptr
      type(c_ptr), value :: context
      type(c_ptr), value :: size
    end subroutine

    function c_oh_context_map_particle_to_neighbor(context, x, y, z, ps) &
        bind(C, name="oh_fortran_context_map_particle_to_neighbor") result(dst)
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: x
      type(c_ptr), value :: y
      type(c_ptr), value :: z
      integer(c_int), value :: ps
      integer(c_int) :: dst
    end function

    function c_oh_context_map_particle_to_subdomain(context, x, y, z) &
        bind(C, name="oh_fortran_context_map_particle_to_subdomain") &
        result(dst)
      import :: c_ptr, c_int, c_double
      type(c_ptr), value :: context
      real(c_double), value :: x
      real(c_double), value :: y
      real(c_double), value :: z
      integer(c_int) :: dst
    end function

    subroutine c_oh_context_bcast_field(context, pfld, sfld, ftype) &
        bind(C, name="oh_fortran_context_bcast_field")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pfld
      type(c_ptr), value :: sfld
      integer(c_int), value :: ftype
    end subroutine

    subroutine c_oh_context_reduce_field(context, pfld, sfld, ftype) &
        bind(C, name="oh_fortran_context_reduce_field")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pfld
      type(c_ptr), value :: sfld
      integer(c_int), value :: ftype
    end subroutine

    subroutine c_oh_context_allreduce_field(context, pfld, sfld, ftype) &
        bind(C, name="oh_fortran_context_allreduce_field")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pfld
      type(c_ptr), value :: sfld
      integer(c_int), value :: ftype
    end subroutine

    subroutine c_oh_context_exchange_borders(context, pfld, sfld, ctype, &
                                             bcast) &
        bind(C, name="oh_fortran_context_exchange_borders")
      import :: c_ptr, c_int
      type(c_ptr), value :: context
      type(c_ptr), value :: pfld
      type(c_ptr), value :: sfld
      integer(c_int), value :: ctype
      integer(c_int), value :: bcast
    end subroutine

    subroutine c_oh2_init_raw(sdid, nspec, maxfrac, nphgram, totalp, pbuf, &
        pbase, maxlocalp, mycomm, nbor, pcoord, stats, repiter, verbose) &
        bind(C, name="oh_fortran_oh2_init_raw")
      import :: c_ptr, c_int
      type(c_ptr), value :: sdid
      integer(c_int), value :: nspec
      integer(c_int), value :: maxfrac
      type(c_ptr), value :: nphgram
      type(c_ptr), value :: totalp
      type(c_ptr) :: pbuf
      type(c_ptr), value :: pbase
      integer(c_int), value :: maxlocalp
      type(c_ptr), value :: mycomm
      type(c_ptr), value :: nbor
      type(c_ptr), value :: pcoord
      integer(c_int), value :: stats
      integer(c_int), value :: repiter
      integer(c_int), value :: verbose
    end subroutine

    subroutine c_oh3_init_raw(sdid, nspec, maxfrac, nphgram, totalp, pbuf, &
        pbase, maxlocalp, mycomm, nbor, pcoord, sdoms, scoord, nbound, &
        bcond, bounds, ftypes, cfields, ctypes, fsizes, stats, repiter, &
        verbose) bind(C, name="oh_fortran_oh3_init_raw")
      import :: c_ptr, c_int
      type(c_ptr), value :: sdid
      integer(c_int), value :: nspec
      integer(c_int), value :: maxfrac
      type(c_ptr), value :: nphgram
      type(c_ptr), value :: totalp
      type(c_ptr) :: pbuf
      type(c_ptr), value :: pbase
      integer(c_int), value :: maxlocalp
      type(c_ptr), value :: mycomm
      type(c_ptr), value :: nbor
      type(c_ptr), value :: pcoord
      type(c_ptr), value :: sdoms
      type(c_ptr), value :: scoord
      integer(c_int), value :: nbound
      type(c_ptr), value :: bcond
      type(c_ptr), value :: bounds
      type(c_ptr), value :: ftypes
      type(c_ptr), value :: cfields
      type(c_ptr), value :: ctypes
      type(c_ptr), value :: fsizes
      integer(c_int), value :: stats
      integer(c_int), value :: repiter
      integer(c_int), value :: verbose
    end subroutine

    function c_oh_particle_adapter_create_byte(stride, adapter) &
        bind(C, name="oh_fortran_particle_adapter_create_byte") result(ierr)
      import :: c_ptr, c_int, c_size_t
      integer(c_size_t), value :: stride
      type(c_ptr) :: adapter
      integer(c_int) :: ierr
    end function

    subroutine c_oh_particle_adapter_destroy(adapter) &
        bind(C, name="oh_fortran_particle_adapter_destroy")
      import :: c_ptr
      type(c_ptr), value :: adapter
    end subroutine

    function c_oh_particle_adapter_validate(adapter) &
        bind(C, name="oh_fortran_particle_adapter_validate") result(valid)
      import :: c_ptr, c_int
      type(c_ptr), value :: adapter
      integer(c_int) :: valid
    end function

    subroutine c_oh_particle_adapter_set_mpi_type(adapter, fortran_type) &
        bind(C, name="oh_fortran_particle_adapter_set_mpi_type")
      import :: c_ptr, c_int
      type(c_ptr), value :: adapter
      integer(c_int), value :: fortran_type
    end subroutine

    subroutine c_oh_particle_adapter_set_species_base(adapter, species_base) &
        bind(C, name="oh_fortran_particle_adapter_set_species_base")
      import :: c_ptr, c_int
      type(c_ptr), value :: adapter
      integer(c_int), value :: species_base
    end subroutine

    subroutine c_oh_particle_adapter_use_int_fields(adapter, region_offset, &
                                                    species_offset) &
        bind(C, name="oh_fortran_particle_adapter_use_int_fields")
      import :: c_ptr, c_size_t
      type(c_ptr), value :: adapter
      integer(c_size_t), value :: region_offset
      integer(c_size_t), value :: species_offset
    end subroutine

    subroutine c_oh_particle_adapter_use_single_species_int_region( &
        adapter, region_offset) &
        bind(C, name="oh_fortran_particle_adapter_use_single_species_int_region")
      import :: c_ptr, c_size_t
      type(c_ptr), value :: adapter
      integer(c_size_t), value :: region_offset
    end subroutine

    subroutine c_oh_particle_adapter_use_integer_fields(adapter, &
        region_offset, region_size, species_offset, species_size) &
        bind(C, name="oh_fortran_particle_adapter_use_integer_fields")
      import :: c_ptr, c_size_t
      type(c_ptr), value :: adapter
      integer(c_size_t), value :: region_offset
      integer(c_size_t), value :: region_size
      integer(c_size_t), value :: species_offset
      integer(c_size_t), value :: species_size
    end subroutine

    subroutine c_oh_particle_adapter_use_single_species_integer_region( &
        adapter, region_offset, region_size) &
        bind(C, &
          name="oh_fortran_particle_adapter_use_single_species_integer_region")
      import :: c_ptr, c_size_t
      type(c_ptr), value :: adapter
      integer(c_size_t), value :: region_offset
      integer(c_size_t), value :: region_size
    end subroutine

    subroutine c_oh_particle_adapter_use_position_fields(adapter, x_offset, &
                                                         y_offset, z_offset) &
        bind(C, name="oh_fortran_particle_adapter_use_position_fields")
      import :: c_ptr, c_size_t
      type(c_ptr), value :: adapter
      integer(c_size_t), value :: x_offset
      integer(c_size_t), value :: y_offset
      integer(c_size_t), value :: z_offset
    end subroutine

    subroutine c_oh_particle_adapter_use_level3_position_fields(adapter, &
        x_offset, y_offset, z_offset) &
        bind(C, name="oh_fortran_particle_adapter_use_level3_position_fields")
      import :: c_ptr, c_size_t
      type(c_ptr), value :: adapter
      integer(c_size_t), value :: x_offset
      integer(c_size_t), value :: y_offset
      integer(c_size_t), value :: z_offset
    end subroutine

    subroutine c_oh_particle_adapter_set_callbacks(adapter, get_region, &
        set_region, get_species, map_to_neighbor, map_to_subdomain) &
        bind(C, name="oh_fortran_particle_adapter_set_callbacks")
      import :: c_ptr, c_funptr
      type(c_ptr), value :: adapter
      type(c_funptr), value :: get_region
      type(c_funptr), value :: set_region
      type(c_funptr), value :: get_species
      type(c_funptr), value :: map_to_neighbor
      type(c_funptr), value :: map_to_subdomain
    end subroutine

    function c_oh_particle_field_offset(base, field) &
        bind(C, name="oh_fortran_particle_field_offset") result(offset)
      import :: c_ptr, c_size_t
      type(c_ptr), value :: base
      type(c_ptr), value :: field
      integer(c_size_t) :: offset
    end function
  end interface

contains

  subroutine oh_context_create(context, fortran_comm, ierr)
    type(oh_context_handle), intent(out) :: context
    integer(c_int), intent(in) :: fortran_comm
    integer(c_int), intent(out) :: ierr

    context%ptr = c_null_ptr
    ierr = c_oh_context_create(fortran_comm, context%ptr)
  end subroutine

  subroutine oh_context_destroy(context)
    type(oh_context_handle), intent(inout) :: context

    call c_oh_context_destroy(context%ptr)
    context%ptr = c_null_ptr
  end subroutine

  subroutine oh_context_configure_particles(context, nspec, maxfrac)
    type(oh_context_handle), intent(in) :: context
    integer(c_int), intent(in) :: nspec
    integer(c_int), intent(in) :: maxfrac

    call c_oh_context_configure_particles(context%ptr, nspec, maxfrac)
  end subroutine

  function oh_default_context() result(context)
    type(oh_context_handle) :: context
    context%ptr = c_oh_default_context()
  end function

  logical function oh_context_associated(context)
    type(oh_context_handle), intent(in) :: context
    oh_context_associated = c_associated(context%ptr)
  end function

  logical function oh_particle_adapter_associated(adapter)
    type(oh_particle_adapter_handle), intent(in) :: adapter
    oh_particle_adapter_associated = c_associated(adapter%ptr)
  end function

  subroutine oh_context_set_region_weights(context, weights)
    type(oh_context_handle), intent(in) :: context
    real(c_double), target, intent(in), optional :: weights(:)
    type(c_ptr) :: weights_ptr
    integer(c_int) :: weight_count

    weights_ptr = c_null_ptr
    weight_count = -1_c_int
    if (present(weights)) then
      weight_count = int(size(weights), c_int)
      if (size(weights) > 0) weights_ptr = c_loc(weights(1))
    end if
    call c_oh_context_set_region_weights(context%ptr, weights_ptr, weight_count)
  end subroutine

  subroutine oh_context_set_particle_mpi_type(context, fortran_type)
    type(oh_context_handle), intent(in) :: context
    integer(c_int), intent(in) :: fortran_type
    call c_oh_context_set_particle_mpi_type(context%ptr, fortran_type)
  end subroutine

  subroutine oh_context_set_particle_adapter(context, adapter)
    type(oh_context_handle), intent(in) :: context
    type(oh_particle_adapter_handle), intent(in), optional :: adapter
    type(c_ptr) :: adapter_ptr

    adapter_ptr = c_null_ptr
    if (present(adapter)) adapter_ptr = adapter%ptr
    call c_oh_context_set_particle_adapter(context%ptr, adapter_ptr)
  end subroutine

  subroutine oh_context_bind_particles(context, particles, maxlocalp, ownership)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(inout) :: particles
    integer(c_int), intent(in) :: maxlocalp
    integer(c_int), intent(in) :: ownership

    particles = c_oh_context_bind_particles(context%ptr, particles, &
                                            maxlocalp, ownership)
  end subroutine

  subroutine oh_context_unbind_particles(context)
    type(oh_context_handle), intent(in) :: context

    call c_oh_context_unbind_particles(context%ptr)
  end subroutine

  subroutine oh_context_bind_region_ids(context, sdid, ownership)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(inout) :: sdid
    integer(c_int), intent(in) :: ownership

    sdid = c_oh_context_bind_region_ids(context%ptr, sdid, ownership)
  end subroutine

  subroutine oh_context_unbind_region_ids(context)
    type(oh_context_handle), intent(in) :: context

    call c_oh_context_unbind_region_ids(context%ptr)
  end subroutine

  subroutine oh_context_get_region_ids(context, sdid)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: sdid

    call c_oh_context_get_region_ids(context%ptr, sdid)
  end subroutine

  subroutine oh_context_bind_particle_accounting(context, nphgram, totalp, &
                                                 pbase, ownership)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(inout) :: nphgram
    type(c_ptr), intent(inout) :: totalp
    type(c_ptr), intent(inout) :: pbase
    integer(c_int), intent(in) :: ownership

    call c_oh_context_bind_particle_accounting(context%ptr, nphgram, &
                                               totalp, pbase, ownership)
  end subroutine

  subroutine oh_context_unbind_particle_accounting(context)
    type(oh_context_handle), intent(in) :: context

    call c_oh_context_unbind_particle_accounting(context%ptr)
  end subroutine

  integer(c_int) function oh_context_max_local_particles_for_capacity( &
      context, global_particle_limit, capacity_percent, min_margin)
    type(oh_context_handle), intent(in) :: context
    integer(c_long_long), intent(in) :: global_particle_limit
    integer(c_int), intent(in) :: capacity_percent
    integer(c_int), intent(in) :: min_margin

    oh_context_max_local_particles_for_capacity = &
      c_oh_context_max_local_particles_for_capacity( &
        context%ptr, global_particle_limit, capacity_percent, min_margin)
  end function

  subroutine oh_context_configure_level3(context, pcoord, sdoms, scoord, &
                                         nbound, bcond, bounds, ftypes, &
                                         cfields, ctypes, fsizes)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pcoord
    type(c_ptr), intent(in), value :: sdoms
    type(c_ptr), intent(in), value :: scoord
    integer(c_int), intent(in) :: nbound
    type(c_ptr), intent(in), value :: bcond
    type(c_ptr), intent(in), value :: bounds
    type(c_ptr), intent(in), value :: ftypes
    type(c_ptr), intent(in), value :: cfields
    type(c_ptr), intent(in), value :: ctypes
    type(c_ptr), intent(in), value :: fsizes

    call c_oh_context_configure_level3(context%ptr, pcoord, sdoms, scoord, &
                                       nbound, bcond, bounds, ftypes, &
                                       cfields, ctypes, fsizes)
  end subroutine

  subroutine oh_context_configure_level3_legacy(context, pcoord, sdoms, &
                                                scoord, nbound, bcond, &
                                                bounds, ftypes, cfields, &
                                                ctypes, fsizes)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pcoord
    type(c_ptr), intent(in), value :: sdoms
    type(c_ptr), intent(in), value :: scoord
    integer(c_int), intent(in) :: nbound
    type(c_ptr), intent(in), value :: bcond
    type(c_ptr), intent(in), value :: bounds
    type(c_ptr), intent(in), value :: ftypes
    type(c_ptr), intent(in), value :: cfields
    type(c_ptr), intent(in), value :: ctypes
    type(c_ptr), intent(in), value :: fsizes

    call c_oh_context_configure_level3_legacy(context%ptr, pcoord, sdoms, &
                                              scoord, nbound, bcond, &
                                              bounds, ftypes, cfields, &
                                              ctypes, fsizes)
  end subroutine

  integer(c_int) function oh_context_transbound1(context, currmode, stats)
    type(oh_context_handle), intent(in) :: context
    integer(c_int), intent(in) :: currmode
    integer(c_int), intent(in) :: stats
    oh_context_transbound1 = c_oh_context_transbound1(context%ptr, currmode, &
                                                      stats)
  end function

  integer(c_int) function oh_context_transbound2(context, currmode, stats)
    type(oh_context_handle), intent(in) :: context
    integer(c_int), intent(in) :: currmode
    integer(c_int), intent(in) :: stats
    oh_context_transbound2 = c_oh_context_transbound2(context%ptr, currmode, &
                                                      stats)
  end function

  integer(c_int) function oh_context_transbound3(context, currmode, stats)
    type(oh_context_handle), intent(in) :: context
    integer(c_int), intent(in) :: currmode
    integer(c_int), intent(in) :: stats
    oh_context_transbound3 = c_oh_context_transbound3(context%ptr, currmode, &
                                                      stats)
  end function

  subroutine oh_context_broadcast(context, pbuf, sbuf, pcount, scount, ptype, &
                                  stype)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pbuf
    type(c_ptr), intent(in), value :: sbuf
    integer(c_int), intent(in) :: pcount, scount, ptype, stype
    call c_oh_context_broadcast(context%ptr, pbuf, sbuf, pcount, scount, &
                                ptype, stype)
  end subroutine

  subroutine oh_context_all_reduce(context, pbuf, sbuf, pcount, scount, &
                                   ptype, stype, pop, sop)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pbuf
    type(c_ptr), intent(in), value :: sbuf
    integer(c_int), intent(in) :: pcount, scount, ptype, stype, pop, sop
    call c_oh_context_all_reduce(context%ptr, pbuf, sbuf, pcount, scount, &
                                 ptype, stype, pop, sop)
  end subroutine

  subroutine oh_context_reduce(context, pbuf, sbuf, pcount, scount, ptype, &
                               stype, pop, sop)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pbuf
    type(c_ptr), intent(in), value :: sbuf
    integer(c_int), intent(in) :: pcount, scount, ptype, stype, pop, sop
    call c_oh_context_reduce(context%ptr, pbuf, sbuf, pcount, scount, ptype, &
                             stype, pop, sop)
  end subroutine

  subroutine oh_context_set_total_particles(context)
    type(oh_context_handle), intent(in) :: context
    call c_oh_context_set_total_particles(context%ptr)
  end subroutine

  subroutine oh_context_inject_particle(context, part)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: part
    call c_oh_context_inject_particle(context%ptr, part)
  end subroutine

  function oh_context_inject_particle_get(context, part) result(copy)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: part
    type(c_ptr) :: copy
    copy = c_oh_context_inject_particle_get(context%ptr, part)
  end function

  subroutine oh_context_remap_injected_particle(context, part)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: part
    call c_oh_context_remap_injected_particle(context%ptr, part)
  end subroutine

  subroutine oh_context_remove_injected_particle(context, part)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: part
    call c_oh_context_remove_injected_particle(context%ptr, part)
  end subroutine

  subroutine oh_context_grid_size(context, grid_size)
    type(oh_context_handle), intent(in) :: context
    real(c_double), target, intent(inout) :: grid_size(:)
    if (size(grid_size) < OH_DIMENSION) &
      error stop "oh_context_grid_size requires at least OH_DIMENSION elements"
    call c_oh_context_grid_size(context%ptr, c_loc(grid_size(1)))
  end subroutine

  integer(c_int) function oh_context_map_particle_to_neighbor(context, x, y, &
                                                              z, ps)
    type(oh_context_handle), intent(in) :: context
    real(c_double), target, intent(inout) :: x
    real(c_double), target, intent(inout), optional :: y
    real(c_double), target, intent(inout), optional :: z
    integer(c_int), intent(in) :: ps
    type(c_ptr) :: y_ptr, z_ptr

    y_ptr = c_null_ptr
    z_ptr = c_null_ptr
#if OH_DIMENSION >= 2
    if (.not. present(y)) &
      error stop "oh_context_map_particle_to_neighbor requires y coordinate"
#endif
#if OH_DIMENSION >= 3
    if (.not. present(z)) &
      error stop "oh_context_map_particle_to_neighbor requires z coordinate"
#endif
    if (present(y)) y_ptr = c_loc(y)
    if (present(z)) z_ptr = c_loc(z)
    oh_context_map_particle_to_neighbor = &
      c_oh_context_map_particle_to_neighbor(context%ptr, c_loc(x), y_ptr, &
                                            z_ptr, ps)
  end function

  integer(c_int) function oh_context_map_particle_to_subdomain(context, x, y, z)
    type(oh_context_handle), intent(in) :: context
    real(c_double), intent(in) :: x
    real(c_double), intent(in), optional :: y
    real(c_double), intent(in), optional :: z
    real(c_double) :: y_value, z_value

    y_value = 0.0_c_double
    z_value = 0.0_c_double
#if OH_DIMENSION >= 2
    if (.not. present(y)) &
      error stop "oh_context_map_particle_to_subdomain requires y coordinate"
#endif
#if OH_DIMENSION >= 3
    if (.not. present(z)) &
      error stop "oh_context_map_particle_to_subdomain requires z coordinate"
#endif
    if (present(y)) y_value = y
    if (present(z)) z_value = z
    oh_context_map_particle_to_subdomain = &
      c_oh_context_map_particle_to_subdomain(context%ptr, x, y_value, z_value)
  end function

  subroutine oh_context_bcast_field(context, pfld, sfld, ftype)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pfld
    type(c_ptr), intent(in), value :: sfld
    integer(c_int), intent(in) :: ftype
    call c_oh_context_bcast_field(context%ptr, pfld, sfld, ftype)
  end subroutine

  subroutine oh_context_reduce_field(context, pfld, sfld, ftype)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pfld
    type(c_ptr), intent(in), value :: sfld
    integer(c_int), intent(in) :: ftype
    call c_oh_context_reduce_field(context%ptr, pfld, sfld, ftype)
  end subroutine

  subroutine oh_context_allreduce_field(context, pfld, sfld, ftype)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pfld
    type(c_ptr), intent(in), value :: sfld
    integer(c_int), intent(in) :: ftype
    call c_oh_context_allreduce_field(context%ptr, pfld, sfld, ftype)
  end subroutine

  subroutine oh_context_exchange_borders(context, pfld, sfld, ctype, bcast)
    type(oh_context_handle), intent(in) :: context
    type(c_ptr), intent(in), value :: pfld
    type(c_ptr), intent(in), value :: sfld
    integer(c_int), intent(in) :: ctype, bcast
    call c_oh_context_exchange_borders(context%ptr, pfld, sfld, ctype, bcast)
  end subroutine

  subroutine oh2_init_raw(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, &
                          maxlocalp, mycomm, nbor, pcoord, stats, repiter, &
                          verbose)
    ! pbuf is an inout pointer slot: c_null_ptr as the value requests legacy
    ! allocation, but sdid/nphgram/totalp/pbase/pcoord must be real storage.
    type(c_ptr), intent(in), value :: sdid
    integer(c_int), intent(in) :: nspec
    integer(c_int), intent(in) :: maxfrac
    type(c_ptr), intent(in), value :: nphgram
    type(c_ptr), intent(in), value :: totalp
    type(c_ptr), intent(inout) :: pbuf
    type(c_ptr), intent(in), value :: pbase
    integer(c_int), intent(in) :: maxlocalp
    type(c_ptr), intent(in), value :: mycomm
    type(c_ptr), intent(in), value :: nbor
    type(c_ptr), intent(in), value :: pcoord
    integer(c_int), intent(in) :: stats
    integer(c_int), intent(in) :: repiter
    integer(c_int), intent(in) :: verbose
    call c_oh2_init_raw(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, &
                        maxlocalp, mycomm, nbor, pcoord, stats, repiter, &
                        verbose)
  end subroutine

  subroutine oh3_init_raw(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, &
                          maxlocalp, mycomm, nbor, pcoord, sdoms, scoord, &
                          nbound, bcond, bounds, ftypes, cfields, ctypes, &
                          fsizes, stats, repiter, verbose)
    ! pbuf is an inout pointer slot: c_null_ptr as the value requests legacy
    ! allocation, but sdid/nphgram/totalp/pbase/pcoord must be real storage.
    type(c_ptr), intent(in), value :: sdid
    integer(c_int), intent(in) :: nspec
    integer(c_int), intent(in) :: maxfrac
    type(c_ptr), intent(in), value :: nphgram
    type(c_ptr), intent(in), value :: totalp
    type(c_ptr), intent(inout) :: pbuf
    type(c_ptr), intent(in), value :: pbase
    integer(c_int), intent(in) :: maxlocalp
    type(c_ptr), intent(in), value :: mycomm
    type(c_ptr), intent(in), value :: nbor
    type(c_ptr), intent(in), value :: pcoord
    type(c_ptr), intent(in), value :: sdoms
    type(c_ptr), intent(in), value :: scoord
    integer(c_int), intent(in) :: nbound
    type(c_ptr), intent(in), value :: bcond
    type(c_ptr), intent(in), value :: bounds
    type(c_ptr), intent(in), value :: ftypes
    type(c_ptr), intent(in), value :: cfields
    type(c_ptr), intent(in), value :: ctypes
    type(c_ptr), intent(in), value :: fsizes
    integer(c_int), intent(in) :: stats
    integer(c_int), intent(in) :: repiter
    integer(c_int), intent(in) :: verbose
    call c_oh3_init_raw(sdid, nspec, maxfrac, nphgram, totalp, pbuf, pbase, &
                        maxlocalp, mycomm, nbor, pcoord, sdoms, scoord, &
                        nbound, bcond, bounds, ftypes, cfields, ctypes, &
                        fsizes, stats, repiter, verbose)
  end subroutine

  subroutine oh_particle_adapter_create_byte(adapter, stride, ierr)
    type(oh_particle_adapter_handle), intent(out) :: adapter
    integer(c_size_t), intent(in) :: stride
    integer(c_int), intent(out) :: ierr

    adapter%ptr = c_null_ptr
    ierr = c_oh_particle_adapter_create_byte(stride, adapter%ptr)
  end subroutine

  subroutine oh_particle_adapter_destroy(adapter)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    call c_oh_particle_adapter_destroy(adapter%ptr)
    adapter%ptr = c_null_ptr
  end subroutine

  integer(c_int) function oh_particle_adapter_validate(adapter)
    type(oh_particle_adapter_handle), intent(in) :: adapter
    oh_particle_adapter_validate = c_oh_particle_adapter_validate(adapter%ptr)
  end function

  subroutine oh_particle_adapter_set_mpi_type(adapter, fortran_type)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_int), intent(in) :: fortran_type
    call c_oh_particle_adapter_set_mpi_type(adapter%ptr, fortran_type)
  end subroutine

  subroutine oh_particle_adapter_set_species_base(adapter, species_base)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_int), intent(in) :: species_base
    call c_oh_particle_adapter_set_species_base(adapter%ptr, species_base)
  end subroutine

  subroutine oh_particle_adapter_use_int_fields(adapter, region_offset, &
                                                species_offset)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_size_t), intent(in) :: region_offset, species_offset
    call c_oh_particle_adapter_use_int_fields(adapter%ptr, region_offset, &
                                              species_offset)
  end subroutine

  subroutine oh_particle_adapter_use_single_species_int_region(adapter, &
                                                               region_offset)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_size_t), intent(in) :: region_offset
    call c_oh_particle_adapter_use_single_species_int_region(adapter%ptr, &
                                                             region_offset)
  end subroutine

  subroutine oh_particle_adapter_use_integer_fields(adapter, region_offset, &
      region_size, species_offset, species_size)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_size_t), intent(in) :: region_offset, region_size
    integer(c_size_t), intent(in) :: species_offset, species_size
    call c_oh_particle_adapter_use_integer_fields(adapter%ptr, region_offset, &
      region_size, species_offset, species_size)
  end subroutine

  subroutine oh_particle_adapter_use_single_species_integer_region(adapter, &
      region_offset, region_size)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_size_t), intent(in) :: region_offset, region_size
    call c_oh_particle_adapter_use_single_species_integer_region(adapter%ptr, &
      region_offset, region_size)
  end subroutine

  subroutine oh_particle_adapter_use_position_fields(adapter, x_offset, &
                                                     y_offset, z_offset)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_size_t), intent(in) :: x_offset, y_offset, z_offset
    call c_oh_particle_adapter_use_position_fields(adapter%ptr, x_offset, &
                                                   y_offset, z_offset)
  end subroutine

  subroutine oh_particle_adapter_use_level3_position_fields(adapter, x_offset, &
                                                            y_offset, z_offset)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    integer(c_size_t), intent(in) :: x_offset, y_offset, z_offset
    call c_oh_particle_adapter_use_level3_position_fields(adapter%ptr, &
      x_offset, y_offset, z_offset)
  end subroutine

  subroutine oh_particle_adapter_set_callbacks(adapter, get_region, &
      set_region, get_species, map_to_neighbor, map_to_subdomain)
    type(oh_particle_adapter_handle), intent(inout) :: adapter
    type(c_funptr), intent(in), value :: get_region
    type(c_funptr), intent(in), value :: set_region
    type(c_funptr), intent(in), value :: get_species
    type(c_funptr), intent(in), value :: map_to_neighbor
    type(c_funptr), intent(in), value :: map_to_subdomain
    call c_oh_particle_adapter_set_callbacks(adapter%ptr, get_region, &
      set_region, get_species, map_to_neighbor, map_to_subdomain)
  end subroutine

  integer(c_size_t) function oh_particle_field_offset(base, field)
    type(c_ptr), intent(in), value :: base
    type(c_ptr), intent(in), value :: field
    oh_particle_field_offset = c_oh_particle_field_offset(base, field)
  end function

  subroutine oh_set_particle_adapter(adapter)
    type(oh_particle_adapter_handle), intent(in), optional :: adapter
    type(oh_context_handle) :: context

    context = oh_default_context()
    if (present(adapter)) then
      call oh_context_set_particle_adapter(context, adapter)
    else
      call oh_context_set_particle_adapter(context)
    end if
  end subroutine
end module
