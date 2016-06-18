#!/usr/bin/env bash

set -euo pipefail

# Runs SCRAM to gather and compare performance profile logs.
# The script terminates hard
# if SCRAM fails to process any input.
# It is assumed that SCRAM runs without issues.

readonly CHECK_PREFIX="check_"  # The prefix for logs generated for checking.
readonly ALGOS="bdd zbdd mocus"  # Qualitative algorithms to check.
readonly CALCS="bdd rare-event mcub"  # Quantitative algorithms.
readonly GEN_INPUTS="baobab1 baobab2 chinese"  # General inputs.
readonly CEA_CHECK="bdd_cea9601.scram"  # Special input.

diff_flags="-E -Z -w -t --tabsize=2 -y -W 160 --suppress-common-lines"


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
  grep -E -v '(in [[:digit:]]+(\.[[:digit:]]+)?|time:? [[:digit:]]+(\.[[:digit:]]+)?)'
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
  scram --$1 input/Baobab/baobab1*.xml ${default_config} 2>&1 | _filter > \
    logs/${prefix}baobab1.scram
  scram --$1 input/Baobab/baobab2*.xml ${default_config} 2>&1 | _filter > \
    logs/${prefix}baobab2.scram
  scram --$1 input/Chinese/chinese*.xml ${default_config} 2>&1 | _filter > \
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
# Runs Uncertainty analysis inputs.
#
# Globals:
#   CALCS
# Arguments:
#   The prefix for output log files.
# Returns:
#   None
########################################
_run_uncertainty() {
  local config="--uncertainty true --seed 123"
  for calc in ${CALCS}; do
    local prefix="${1}${calc}_"
    scram input/BSCU/BSCU.xml --${calc} ${config} \
      | grep -E '(mean|standard)' > logs/${prefix}BSCU.scram
  done

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
  _run_uncertainty "$1"
}


########################################
# Removes first files
# if it's no differnt than the second.
#
# Globals:
#   diff_flags
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
  diff ${diff_flags} ${1} ${2} && rm ${1} || ret=1
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
#   CALCS
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

  for calc in ${CALCS}; do
    local input_name="${calc}_BSCU.scram"
    _diff_remove "logs/${CHECK_PREFIX}${input_name}" "logs/${input_name}" \
      || ret=1
  done
  return $ret
}

########################################
# Gathers and diffs SCRAM logs.
#
# Globals:
#   CHECK_PREFIX
#   diff_flags
# Arguments:
#   'update' arg for update only.
# Returns:
#   0 for success.
########################################
main() {
  which scram > /dev/null
  [[ -d logs ]] && [[ -d input ]]

  diff ${diff_flags} install.py install.py  || diff_flags=""

  if [[ $# -gt 0 && "$1" == "update" ]]; then
    _gather_logs ""
  else
    _gather_logs "${CHECK_PREFIX}"
    _filter_diffs
  fi
}

main "$@"
