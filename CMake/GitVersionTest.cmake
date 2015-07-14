#ckwg +4
# Copyright 2010, 2013 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

find_package(Git)

# git_verify_commit(<source_dir> <refspec>)
#
# - source_dir : location of the source code under git control
# - refspec    : refspec of commit to check
#
# Sets the in the parent scope:
# - valid_commit : returns YES if the commit exists or NO otherwise
#
# Tests whether git commit <refspec> exists in the repository
# 
function(git_verify_commit _src_dir _refspec)
  set(valid_commit NO PARENT_SCOPE)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --is-inside-work-tree
                    WORKING_DIRECTORY ${_src_dir}
                    RESULT_VARIABLE is_git_repo
                    OUTPUT_QUIET
                    ERROR_QUIET)
    if (NOT is_git_repo)
      set(valid_repo YES PARENT_SCOPE)
    else ()
      return()
    endif ()
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --verify ${_refspec}^0
                    WORKING_DIRECTORY ${_src_dir}
                    RESULT_VARIABLE git_results
                    OUTPUT_QUIET
                    ERROR_QUIET)
    if(NOT git_results)
      set(valid_commit YES PARENT_SCOPE)
    endif()
  else()
    message(WARNING "Git command not found")
  endif()
endfunction(git_verify_commit)


# git_merge_base(<source_dir> <refspec1> <refspec2>)
#
# - source_dir : location of the source code under git control
# - refspec1   : refspec of commit 1
# - refspec2   : refspec of commit 2
#
# Sets the in the parent scope:
# - merge_base : the result of running git merge-base <refspec1> <refspec2>
#
# Finds a common commit hash in the history reachable from both 
# <refspec1> and <refspec2>.
# 
function(git_merge_base _src_dir _refspec1 _refspec2)
  unset(_result)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} merge-base ${_refspec1} ${_refspec2}
                    WORKING_DIRECTORY ${_src_dir}
                    RESULT_VARIABLE git_results
                    OUTPUT_VARIABLE git_output
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT git_results)
      set(merge_base ${git_output} PARENT_SCOPE)
    else()
      message(WARNING "git merge-base failed with result ${git_results}")
    endif()
  else()
    message(WARNING "Git command not found")
  endif()
endfunction(git_merge_base)



# git_commit_reachable(<source_dir> <refspec_prev> <refspec_curr>)
#
# - source_dir   : location of the source code under git control
# - refspec_prev : refspec of the previous commit
# - refspec_curr : refspec of the current commit
#
# Sets the in the parent scope:
# - commit_reachable : returns YES if reachable or NO otherwise
#
# Tests whether git commit <refspec_prev> is reachable from 
# <refspec_curr>.
# 
function(git_commit_reachable _src_dir _refspec1 _refspec2)
  set(commit_reachable NO PARENT_SCOPE)
  git_merge_base(${_src_dir} ${_refspec1} ${_refspec1})
  set(hash1 ${merge_base})
  git_merge_base(${_src_dir} ${_refspec1} ${_refspec2})
  if(${merge_base} STREQUAL ${hash1})
    set(commit_reachable YES PARENT_SCOPE)
  endif()
endfunction(git_commit_reachable)



# check_reachable_git_hash(<project_name> <source_dir> <reachable_hash> [QUIET])
#
# - project_name   : name of the project contained in <source_dir>
# - source_dir     : location of the source code under git control
# - reachable_hash : git commit hash of the reachable commit
#
# Generate a warning if an outdated state of the project is being used.
# Verifies that <reachable_hash> is be reachable in the history from the 
# HEAD git commit of the project.  Prints an affirmative status message on
# success unless QUIET is specified.
#
function(check_reachable_git_hash _project_name _src_dir _reachable_hash)

  if(NOT GIT_FOUND)
    message(WARNING "Git command not found, unable to check reachable hash")
    return()
  endif()
  git_verify_commit(${_src_dir} ${_reachable_hash})
  if(NOT valid_repo)
    message(WARNING "Unable to verify git commit (${_reachable_hash}) for ${_project_name} "
                    "because it is not a git repository.  The assumption will be that it is valid.")
    return()
  endif ()
  if(NOT valid_commit)
    message(SEND_ERROR "Required ${_project_name} git commit (${_reachable_hash}) "
                       "is not valid.  Try a 'git fetch' in ${_src_dir}.")
    return()
  endif()

  git_commit_reachable(${_src_dir} ${_reachable_hash} "HEAD")
  if(commit_reachable)
    if(NOT ARGV MATCHES "QUIET")
      message(STATUS "Required ${_project_name} commit (${_reachable_hash}) "
                     "is reachable from the current checkout.")
    endif()
  else()
    message(SEND_ERROR "Required ${_project_name} commit (${_reachable_hash}) "
                       "is not reachable from the current checkout "
                       "in ${_src_dir}")
  endif()
endfunction(check_reachable_git_hash)



# check_required_git_hash(<project_name> <source_dir> <required_hash> [QUIET])
#
# - project_name  : name of the project contained in <source_dir>
# - source_dir    : location of the source code under git control
# - required_hash : required git commit hash
#
# Generate a warning if an outdated state of the project is being used.
# Verifies that <required_hash> is an exact match to the HEAD git commit 
# of the project.  Prints an affirmative status message on success 
# unless QUIET is specified.
#
function(check_required_git_hash _project_name _src_dir _required_hash)

  if(NOT GIT_FOUND)
    message(WARNING "Git command not found, unable to check required hash")
    return()
  endif()
  git_verify_commit(${_src_dir} ${_required_hash})
  if(NOT valid_repo)
    message(WARNING "Unable to verify git commit (${_reachable_hash}) for ${_project_name} "
                    "because it is not a git repository.  The assumption will be that it is valid.")
    return()
  endif ()
  if(NOT valid_commit)
    message(SEND_ERROR "Required ${_project_name} git commit (${_required_hash}) "
                       "is not valid.  Try a 'git fetch' in ${_src_dir}.")
    return()
  endif()

  git_merge_base(${_src_dir} HEAD HEAD)
  set(head_hash ${merge_base})
  git_merge_base(${_src_dir} ${_required_hash} ${_required_hash})
  if(${merge_base} STREQUAL ${head_hash})
    if(NOT ARGV MATCHES "QUIET")
      message(STATUS "Required ${_project_name} commit (${_required_hash}) "
                     "matches the current checkout.")
    endif()
  else()
    message(SEND_ERROR "Required ${_project_name} commit (${_required_hash}) "
                       "does not match HEAD commit (${head_hash}) "
                       "in ${_src_dir}")
  endif()
endfunction(check_required_git_hash)



