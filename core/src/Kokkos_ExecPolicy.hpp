/*
//@HEADER
// ************************************************************************
// 
//                        Kokkos v. 2.0
//              Copyright (2014) Sandia Corporation
// 
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact  H. Carter Edwards (hcedwar@sandia.gov)
// 
// ************************************************************************
//@HEADER
*/

#ifndef KOKKOS_EXECPOLICY_HPP
#define KOKKOS_EXECPOLICY_HPP

#include <Kokkos_Core_fwd.hpp>
#include <impl/Kokkos_Traits.hpp>
#include <impl/Kokkos_StaticAssert.hpp>
#include <impl/Kokkos_Tags.hpp>

//----------------------------------------------------------------------------

namespace Kokkos {

//Schedules for Execution Policies
struct Static {
  typedef Static schedule_type;
};

struct Dynamic {
  typedef Dynamic schedule_type;
};

template<class ScheduleType>
struct Schedule {
  typedef ScheduleType schedule_type;
};

template<class Arg>
struct is_schedule_type {
  enum { value = 0};
};

template<class ScheduleType>
struct is_schedule_type<Schedule<ScheduleType> > {
  enum {value = 1 };
};

//Specif Iteration Index Type
template<typename iType>
struct IterationType {
  typedef iType iteration_type;
};

template<class Arg>
struct is_iteration_type {
  enum { value = 0 };
};

template<typename iType>
struct is_iteration_type<IterationType<iType> > {
  enum { value = 1 };
};

namespace Impl {
//Policy Traits
template<class ... Properties>
struct PolicyTraits;

template<>
struct PolicyTraits<void> {
  typedef void execution_space;
  typedef void schedule_type;
  typedef void iteration_type;
  typedef void tag_type;
};


//Strip off ExecutionSpace
template<class ExecutionSpace, class ... Props>
struct PolicyTraits<typename std::enable_if<is_execution_space<ExecutionSpace>::value >::type,ExecutionSpace,Props ...> {
  static_assert( std::is_same<typename PolicyTraits<void, Props ...>::execution_space, void>::value,
                 "ExecutionPolicy: Only one execution space template argument may be used.");
  typedef ExecutionSpace execution_space;
  typedef typename PolicyTraits<void, Props ...>::schedule_type schedule_type;
  typedef typename PolicyTraits<void, Props ...>::iteration_type iteration_type;
  typedef typename PolicyTraits<void, Props ...>::tag_type tag_type;
};

//Strip off ScheduleType
template<class ScheduleType, class ... Props>
struct PolicyTraits<typename std::enable_if<is_schedule_type<Schedule<ScheduleType> >::value >::type,Schedule<ScheduleType>,Props ...> {
  static_assert( std::is_same<typename PolicyTraits<void, Props ...>::schedule_type, void>::value,
                 "ExecutionPolicy: Only one Schedule<..> template argument may be used.");
  typedef typename PolicyTraits<void, Props ...>::execution_space execution_space;
  typedef ScheduleType schedule_type;
  typedef typename PolicyTraits<void, Props ...>::iteration_type iteration_type;
  typedef typename PolicyTraits<void, Props ...>::tag_type tag_type;
};

//Strip off IterationType
template<typename iType, class ... Props>
struct PolicyTraits<void, IterationType<iType>,Props ...> {
  static_assert( std::is_same<typename PolicyTraits<void, Props ...>::iteration_type, void>::value,
                 "ExecutionPolicy: Only one IterationType<..> template argument may be used.");
  typedef typename PolicyTraits<void, Props ...>::execution_space execution_space;
  typedef typename PolicyTraits<void, Props ...>::schedule_type schedule_type;
  typedef iType iteration_type;
  typedef typename PolicyTraits<void, Props ...>::tag_type tag_type;
};

//Strip off raw IterationType
template<typename iType, class ... Props>
struct PolicyTraits<typename std::enable_if<std::is_integral<iType>::value>::type, iType,Props ...> {
  static_assert( std::is_same<typename PolicyTraits<void, Props ...>::iteration_type, void>::value,
                 "ExecutionPolicy: Only one IterationType<..> template argument may be used.");
  typedef typename PolicyTraits<void, Props ...>::execution_space execution_space;
  typedef typename PolicyTraits<void, Props ...>::schedule_type schedule_type;
  typedef iType iteration_type;
  typedef typename PolicyTraits<void, Props ...>::tag_type tag_type;
};

//Strip off TagType
template<class TagType, class ... Props>
struct PolicyTraits<typename std::enable_if<!is_schedule_type<TagType>::value &&
                                            !is_execution_space<TagType>::value &&
                                            !is_iteration_type<TagType>::value &&
                                            !std::is_integral<TagType>::value 
                                           >::type,
                    TagType,Props ...> {
  static_assert( std::is_same<typename PolicyTraits<void, Props ...>::tag_type, void>::value,
                 "ExecutionPolicy: Only one tag type template argument may be used.");

  typedef typename PolicyTraits<void, Props ...>::execution_space execution_space;
  typedef typename PolicyTraits<void, Props ...>::schedule_type schedule_type;
  typedef typename PolicyTraits<void, Props ...>::iteration_type iteration_type;
  typedef TagType tag_type;
};


template<class ... Props>
struct PolicyTraits {
  typedef typename std::conditional<std::is_same<void, typename PolicyTraits<void, Props ...>::execution_space>::value, 
    Kokkos::DefaultExecutionSpace, typename PolicyTraits<void,Props ...>::execution_space>::type execution_space;
  typedef typename std::conditional<std::is_same<void, typename PolicyTraits<void, Props ...>::schedule_type>::value, 
    Kokkos::Static, typename PolicyTraits<void,Props ...>::schedule_type>::type schedule_type;
  typedef typename std::conditional<std::is_same<void, typename PolicyTraits<void, Props ...>::iteration_type>::value, 
    typename execution_space::size_type, typename PolicyTraits<void,Props ...>::iteration_type>::type iteration_type;
  typedef typename std::conditional<std::is_same<void, typename PolicyTraits<void, Props ...>::tag_type>::value, 
    void, typename PolicyTraits<void,Props ...>::tag_type>::type work_tag;
};

}

struct ChunkSize {
  long chunk_size;
  ChunkSize(const long& value):chunk_size(value) {}
  ChunkSize(const Kokkos::AUTO_t&):chunk_size(-1) {}
};
/** \brief  Execution policy for work over a range of an integral type.
 *
 * Valid template argument options:
 *
 *  With a specified execution space:
 *    < ExecSpace , WorkTag , { IntConst | IntType } >
 *    < ExecSpace , WorkTag , void >
 *    < ExecSpace , { IntConst | IntType } , void >
 *    < ExecSpace , void , void >
 *
 *  With the default execution space:
 *    < WorkTag , { IntConst | IntType } , void >
 *    < WorkTag , void , void >
 *    < { IntConst | IntType } , void , void >
 *    < void , void , void >
 *
 *  IntType  is a fundamental integral type
 *  IntConst is an Impl::integral_constant< IntType , Blocking >
 *
 *  Blocking is the granularity of partitioning the range among threads.
 */
template<class ... Properties>
class RangePolicy: public Impl::PolicyTraits<Properties ... > {
private:

  typedef Impl::PolicyTraits<Properties ... > traits;

  // Default integral type and blocking factor:
  enum { Granularity = 8 };

  // Only accept the integral type if the blocking is a power of two
  static_assert( Impl::is_integral_power_of_two( Granularity )
               , "RangePolicy blocking granularity must be power of two" );

  enum { GranularityMask = typename traits::iteration_type(Granularity) - 1 };

  typename traits::execution_space m_space ;
  typename traits::iteration_type  m_begin ;
  typename traits::iteration_type  m_end ;
  typename traits::iteration_type  m_granularity ;
  typename traits::iteration_type  m_granularity_mask ;
public:

  //! Tag this class as an execution policy
  typedef typename traits::iteration_type member_type ;

  KOKKOS_INLINE_FUNCTION const typename traits::execution_space & space() const { return m_space ; }
  KOKKOS_INLINE_FUNCTION member_type begin() const { return m_begin ; }
  KOKKOS_INLINE_FUNCTION member_type end()   const { return m_end ; }

  inline RangePolicy() : m_space(), m_begin(0), m_end(0) {}

  /** \brief  Total range */
  inline
  RangePolicy( const member_type work_begin
             , const member_type work_end
             )
    : m_space()
    , m_begin( work_begin < work_end ? work_begin : 0 )
    , m_end(   work_begin < work_end ? work_end : 0 )
    , m_granularity(Granularity)
    , m_granularity_mask(m_granularity-1)
    {}

  /** \brief  Total range */
  inline
  RangePolicy( const typename traits::execution_space & work_space
             , const member_type work_begin
             , const member_type work_end
             )
    : m_space( work_space )
    , m_begin( work_begin < work_end ? work_begin : 0 )
    , m_end(   work_begin < work_end ? work_end : 0 )
    , m_granularity(Granularity)
    , m_granularity_mask(m_granularity-1)
    {}

  /** \brief  Total range */
  inline
  RangePolicy( const member_type work_begin
             , const member_type work_end
             , const ChunkSize chunk
             )
    : m_space()
    , m_begin( work_begin < work_end ? work_begin : 0 )
    , m_end(   work_begin < work_end ? work_end : 0 )
    {
      m_granularity = chunk.chunk_size;      
      m_granularity_mask = m_granularity - 1;
    }

  /** \brief  Total range */
  inline
  RangePolicy( const typename traits::execution_space & work_space
             , const member_type work_begin
             , const member_type work_end
             , const ChunkSize chunk
             )
    : m_space( work_space )
    , m_begin( work_begin < work_end ? work_begin : 0 )
    , m_end(   work_begin < work_end ? work_end : 0 )
    {
      m_granularity = chunk.chunk_size;      
      m_granularity_mask = m_granularity - 1;
    }

  public:

     inline member_type chunk_size() const {
       return m_granularity;
     }

     inline RangePolicy set_chunk_size(int concurrency) {
       if(m_granularity > 0)
         return *this;

       member_type new_chunk_size = 1;
       while(new_chunk_size*100*concurrency < m_end-m_begin)
         new_chunk_size *= 2;
       if(new_chunk_size < 128) {
         new_chunk_size = 1;
         while( (new_chunk_size*40*concurrency < m_end-m_begin ) && (new_chunk_size<128) )
           new_chunk_size*=2;
       }
       m_granularity = new_chunk_size;
       m_granularity_mask = m_granularity - 1;
       return *this;
     }

  /** \brief  Subrange for a partition's rank and size.
   *
   *  Typically used to partition a range over a group of threads.
   */
  struct WorkRange {
    typedef typename RangePolicy::work_tag     work_tag ;
    typedef typename RangePolicy::member_type  member_type ;

    KOKKOS_INLINE_FUNCTION member_type begin() const { return m_begin ; }
    KOKKOS_INLINE_FUNCTION member_type end()   const { return m_end ; }

    /** \brief  Subrange for a partition's rank and size.
     *
     *  Typically used to partition a range over a group of threads.
     */
    KOKKOS_INLINE_FUNCTION
    WorkRange( const RangePolicy & range
             , const int part_rank
             , const int part_size
             )
      : m_begin(0), m_end(0)
      {
        if ( part_size ) {
  
          // Split evenly among partitions, then round up to the granularity.
          const member_type work_part =
            ( ( ( ( range.end() - range.begin() ) + ( part_size - 1 ) ) / part_size )
              + range.m_granularity_mask ) & ~member_type(range.m_granularity_mask);

          m_begin = range.begin() + work_part * part_rank ;
          m_end   = m_begin       + work_part ;
  
          if ( range.end() < m_begin ) m_begin = range.end() ;
          if ( range.end() < m_end )   m_end   = range.end() ;
        }
      }
  private:
     member_type m_begin ;
     member_type m_end ;
     WorkRange();
     WorkRange & operator = ( const WorkRange & );
   
  };
};

} // namespace Kokkos

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace Kokkos {

namespace Experimental {

/** \brief Scratch memory request accepting per team and per thread value
 *
 * An instance of this class can be given as the last argument to a 
 * TeamPolicy constructor. It sets the amount of user requested shared
 * memory for the team.
 */

template< class MemorySpace >
class TeamScratchRequest {
  size_t m_per_team;
  size_t m_per_thread;
  
public:
  TeamScratchRequest(size_t per_team_, size_t per_thread_ = 0):
   m_per_team(per_team_), m_per_thread(per_thread_) {
  } 

  size_t per_team() const {
    return m_per_team;
  }
  size_t per_thread() const {
    return m_per_thread;
  }
  size_t total(const size_t team_size) const {
    return m_per_team + m_per_thread * team_size;
  }
}; 

}

/** \brief  Execution policy for parallel work over a league of teams of threads.
 *
 *  The work functor is called for each thread of each team such that
 *  the team's member threads are guaranteed to be concurrent.
 *
 *  The team's threads have access to team shared scratch memory and
 *  team collective operations.
 *
 *  If the WorkTag is non-void then the first calling argument of the
 *  work functor's parentheses operator is 'const WorkTag &'.
 *  This allows a functor to have multiple work member functions.
 *
 *  template argument option with specified execution space:
 *    < ExecSpace , WorkTag >
 *    < ExecSpace , void >
 *
 *  template argument option with default execution space:
 *    < WorkTag , void >
 *    < void , void >
 */
template< class Arg0 = void
        , class Arg1 = void
        , class ExecSpace =
          // If the first argument is not an execution
          // then use the default execution space.
          typename std::conditional
            < Impl::is_execution_space< Arg0 >::value , Arg0
            , Kokkos::DefaultExecutionSpace >::type
        >
class TeamPolicy {
private:

  enum { Arg0_ExecSpace = Impl::is_execution_space< Arg0 >::value };
  enum { Arg1_Void      = Impl::is_same< Arg1 , void >::value };
  enum { ArgOption_OK   = Impl::StaticAssert< ( Arg0_ExecSpace || Arg1_Void ) >::value };

  typedef typename std::conditional< Arg0_ExecSpace , Arg1 , Arg0 >::type WorkTag ;

public:

  //! Tag this class as an execution policy
  typedef TeamPolicy  execution_policy ;
  typedef ExecSpace   execution_space ;
  typedef WorkTag     work_tag ;

  //----------------------------------------
  /** \brief  Query maximum team size for a given functor.
   *
   *  This size takes into account execution space concurrency limitations and
   *  scratch memory space limitations for reductions, team reduce/scan, and
   *  team shared memory.
   */
  template< class FunctorType >
  static int team_size_max( const FunctorType & );

  /** \brief  Query recommended team size for a given functor.
   *
   *  This size takes into account execution space concurrency limitations and
   *  scratch memory space limitations for reductions, team reduce/scan, and
   *  team shared memory.
   */
  template< class FunctorType >
  static int team_size_recommended( const FunctorType & );

  template< class FunctorType >
  static int team_size_recommended( const FunctorType & , const int&);
  //----------------------------------------
  /** \brief  Construct policy with the given instance of the execution space */
  TeamPolicy( const execution_space & , int league_size_request , int team_size_request , int vector_length_request = 1 );

  TeamPolicy( const execution_space & , int league_size_request , const Kokkos::AUTO_t & , int vector_length_request = 1 );

  /** \brief  Construct policy with the default instance of the execution space */
  TeamPolicy( int league_size_request , int team_size_request , int vector_length_request = 1 );

  TeamPolicy( int league_size_request , const Kokkos::AUTO_t & , int vector_length_request = 1 );

  template<class MemorySpace>
  TeamPolicy( int league_size_request , int team_size_request , const Experimental::TeamScratchRequest<MemorySpace>& team_scratch_memory_request );

  template<class MemorySpace>
  TeamPolicy( int league_size_request , const Kokkos::AUTO_t & , const Experimental::TeamScratchRequest<MemorySpace>& team_scratch_memory_request );

  /** \brief  The actual league size (number of teams) of the policy.
   *
   *  This may be smaller than the requested league size due to limitations
   *  of the execution space.
   */
  KOKKOS_INLINE_FUNCTION int league_size() const ;

  /** \brief  The actual team size (number of threads per team) of the policy.
   *
   *  This may be smaller than the requested team size due to limitations
   *  of the execution space.
   */
  KOKKOS_INLINE_FUNCTION int team_size() const ;

  /** \brief  Parallel execution of a functor calls the functor once with
   *          each member of the execution policy.
   */
  struct member_type {

    /** \brief  Handle to the currently executing team shared scratch memory */
    KOKKOS_INLINE_FUNCTION
    typename execution_space::scratch_memory_space team_shmem() const ;

    /** \brief  Rank of this team within the league of teams */
    KOKKOS_INLINE_FUNCTION int league_rank() const ;

    /** \brief  Number of teams in the league */
    KOKKOS_INLINE_FUNCTION int league_size() const ;

    /** \brief  Rank of this thread within this team */
    KOKKOS_INLINE_FUNCTION int team_rank() const ;

    /** \brief  Number of threads in this team */
    KOKKOS_INLINE_FUNCTION int team_size() const ;

    /** \brief  Barrier among the threads of this team */
    KOKKOS_INLINE_FUNCTION void team_barrier() const ;

    /** \brief  Intra-team reduction. Returns join of all values of the team members. */
    template< class JoinOp >
    KOKKOS_INLINE_FUNCTION
    typename JoinOp::value_type team_reduce( const typename JoinOp::value_type
                                           , const JoinOp & ) const ;

    /** \brief  Intra-team exclusive prefix sum with team_rank() ordering.
     *
     *  The highest rank thread can compute the reduction total as
     *    reduction_total = dev.team_scan( value ) + value ;
     */
    template< typename Type >
    KOKKOS_INLINE_FUNCTION Type team_scan( const Type & value ) const ;

    /** \brief  Intra-team exclusive prefix sum with team_rank() ordering
     *          with intra-team non-deterministic ordering accumulation.
     *
     *  The global inter-team accumulation value will, at the end of the
     *  league's parallel execution, be the scan's total.
     *  Parallel execution ordering of the league's teams is non-deterministic.
     *  As such the base value for each team's scan operation is similarly
     *  non-deterministic.
     */
    template< typename Type >
    KOKKOS_INLINE_FUNCTION Type team_scan( const Type & value , Type * const global_accum ) const ;
  };
};

} // namespace Kokkos

namespace Kokkos {

namespace Impl {

template<typename iType, class TeamMemberType>
struct TeamThreadRangeBoundariesStruct {
private:

  KOKKOS_INLINE_FUNCTION static
  iType ibegin( const iType & arg_begin
              , const iType & arg_end
              , const iType & arg_rank
              , const iType & arg_size
              )
    {
      return arg_begin + ( ( arg_end - arg_begin + arg_size - 1 ) / arg_size ) * arg_rank ;
    }

  KOKKOS_INLINE_FUNCTION static
  iType iend( const iType & arg_begin
            , const iType & arg_end
            , const iType & arg_rank
            , const iType & arg_size
            )
    {
      const iType end_ = arg_begin + ( ( arg_end - arg_begin + arg_size - 1 ) / arg_size ) * ( arg_rank + 1 );
      return end_ < arg_end ? end_ : arg_end ;
    }

public:

  typedef iType index_type;
  const iType start;
  const iType end;
  enum {increment = 1};
  const TeamMemberType& thread;

  KOKKOS_INLINE_FUNCTION
  TeamThreadRangeBoundariesStruct( const TeamMemberType& arg_thread
                                , const iType& arg_end
                                )
    : start( ibegin( 0 , arg_end , arg_thread.team_rank() , arg_thread.team_size() ) )
    , end(   iend(   0 , arg_end , arg_thread.team_rank() , arg_thread.team_size() ) )
    , thread( arg_thread )
    {}

  KOKKOS_INLINE_FUNCTION
  TeamThreadRangeBoundariesStruct( const TeamMemberType& arg_thread
                                , const iType& arg_begin
                                , const iType& arg_end
                                )
    : start( ibegin( arg_begin , arg_end , arg_thread.team_rank() , arg_thread.team_size() ) )
    , end(   iend(   arg_begin , arg_end , arg_thread.team_rank() , arg_thread.team_size() ) )
    , thread( arg_thread )
    {}
};

  template<typename iType, class TeamMemberType>
  struct ThreadVectorRangeBoundariesStruct {
    typedef iType index_type;
    enum {start = 0};
    const iType end;
    enum {increment = 1};

    KOKKOS_INLINE_FUNCTION
    ThreadVectorRangeBoundariesStruct (const TeamMemberType& thread, const iType& count):
      end( count )
    {}
  };

  template<class TeamMemberType>
  struct ThreadSingleStruct {
    const TeamMemberType& team_member;
    KOKKOS_INLINE_FUNCTION
    ThreadSingleStruct(const TeamMemberType& team_member_):team_member(team_member_){}
  };

  template<class TeamMemberType>
  struct VectorSingleStruct {
    const TeamMemberType& team_member;
    KOKKOS_INLINE_FUNCTION
    VectorSingleStruct(const TeamMemberType& team_member_):team_member(team_member_){}
  };
} // namespace Impl

/** \brief  Execution policy for parallel work over a threads within a team.
 *
 *  The range is split over all threads in a team. The Mapping scheme depends on the architecture.
 *  This policy is used together with a parallel pattern as a nested layer within a kernel launched
 *  with the TeamPolicy. This variant expects a single count. So the range is (0,count].
 */
template<typename iType, class TeamMemberType>
KOKKOS_INLINE_FUNCTION
Impl::TeamThreadRangeBoundariesStruct<iType,TeamMemberType> TeamThreadRange(const TeamMemberType&, const iType& count);

/** \brief  Execution policy for parallel work over a threads within a team.
 *
 *  The range is split over all threads in a team. The Mapping scheme depends on the architecture.
 *  This policy is used together with a parallel pattern as a nested layer within a kernel launched
 *  with the TeamPolicy. This variant expects a begin and end. So the range is (begin,end].
 */
template<typename iType, class TeamMemberType>
KOKKOS_INLINE_FUNCTION
Impl::TeamThreadRangeBoundariesStruct<iType,TeamMemberType> TeamThreadRange(const TeamMemberType&, const iType& begin, const iType& end);

/** \brief  Execution policy for a vector parallel loop.
 *
 *  The range is split over all vector lanes in a thread. The Mapping scheme depends on the architecture.
 *  This policy is used together with a parallel pattern as a nested layer within a kernel launched
 *  with the TeamPolicy. This variant expects a single count. So the range is (0,count].
 */
template<typename iType, class TeamMemberType>
KOKKOS_INLINE_FUNCTION
Impl::ThreadVectorRangeBoundariesStruct<iType,TeamMemberType> ThreadVectorRange(const TeamMemberType&, const iType& count);

} // namespace Kokkos

#endif /* #define KOKKOS_EXECPOLICY_HPP */

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

