#!/usr/bin/env bash

set -eu

# Runs SCRAM to gather and compare performance profile logs.
# The script terminates hard
# if SCRAM fails to process any input.
# It is assumed that SCRAM runs without issues.

readonly CHECK_PREFIX="check_"  # The prefix for logs generated for checking.
readonly ALGOS="bdd zbdd mocus"  # Algorithms to check.
readonly GEN_INPUTS="baobab1 baobab2 chinese"  # General inputs.
readonly CEA_CHECK="bdd_cea9601.scram"  # Special input.

########################################
# Filters unstable parts of SCRAM logs.
#
# Globals:
#   None
# Arguments:
#   The log output from SCRAM.
# Returns:
#   None
########################################
_filter() {
  grep -P -v '(in \d+(\.\d+)?|time:? \d+(\.\d+)?)'
}

########################################
# Runs general inputs to gather logs.
#
# Globals:
#   None
# Arguments:
#   The name of the algorithm.
#   The prefix for output log files.
# Returns:
#   None
########################################
_run_general_inputs() {
  local default_config="-o /dev/null --verbosity 7"
  local prefix="${2}${1}_"
  scram --$1 input/Baobab/baobab1*.xml $default_config 2>&1 | _filter > \
    logs/${prefix}baobab1.scram
  scram --$1 input/Baobab/baobab2*.xml $default_config 2>&1 | _filter > \
    logs/${prefix}baobab2.scram
  scram --$1 input/Chinese/chinese*.xml $default_config 2>&1 | _filter > \
    logs/${prefix}chinese.scram
}

########################################
# Runs CEA9601 input with BDD only.
#
# Globals:
#   None
# Arguments:
#   The prefix for output log files.
# Returns:
#   None
########################################
_run_cea() {
  local config="-o /dev/null --verbosity 6 -l 4"
  local prefix="${1}bdd_"
  scram --bdd input/CEA9601/CEA9601*.xml $config 2>&1 | _filter > \
    logs/${prefix}cea9601.scram
}

########################################
# Gathers all logs from SCRAM runs.
#
# Globals:
#   None
# Arguments:
#   The prefix for output log files.
# Returns:
#   None
########################################
_gather_logs() {
  _run_general_inputs bdd "$1"
  _run_general_inputs zbdd "$1"
  _run_general_inputs mocus "$1"
  _run_cea "$1"
}


########################################
# Removes first files
# if it's no differnt than the second.
#
# Globals:
#   None
# Arguments:
#   File one.
#   File two.
# Returns:
#   Non-zero if files are different.
########################################
_diff_remove() {
  local ret=0
  echo "================================================================"
  echo ${1} "    |    "${2}
  echo "----------------------------------------------------------------"
  diff ${1} ${2} && rm ${1} || ret=1
  echo -e "================================================================\n"
  return $ret
}

########################################
# Compares generated logs
# with previously generated references.
#
# Globals:
#   CHECK_PREFIX
#   ALGOS
#   GEN_INPUTS
#   CEA_CHECK
# Arguments:
#   None
# Returns:
#   Non-zero if some files differ.
########################################
_filter_diffs() {
  local ret=0
  _diff_remove "logs/${CHECK_PREFIX}${CEA_CHECK}" "logs/${CEA_CHECK}" || ret=1

  for algo in ${ALGOS}; do
    for input in ${GEN_INPUTS}; do
      local input_name="${algo}_${input}.scram"
      _diff_remove "logs/${CHECK_PREFIX}${input_name}" "logs/${input_name}" \
        || ret=1
    done
  done
  return $ret
}

########################################
# Gathers and diffs SCRAM logs.
#
# Globals:
#   CHECK_PREFIX
# Arguments:
#   'update' arg for update only.
# Returns:
#   0 for success.
########################################
main() {
  which scram > /dev/null
  [[ -d logs ]] && [[ -d input ]]

  if [[ $# > 0 && "$1" == "update" ]]; then
    _gather_logs ""
  else
    _gather_logs "${CHECK_PREFIX}"
    _filter_diffs
  fi
}

main "$@"
