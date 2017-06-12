/** @file args.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef ARGS_H
#define ARGS_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef ARGS_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define ARGS_PARSER_PACKAGE "qbinstr"
#endif

#ifndef ARGS_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define ARGS_PARSER_PACKAGE_NAME "qbinstr"
#endif

#ifndef ARGS_PARSER_VERSION
/** @brief the program version */
#define ARGS_PARSER_VERSION "0.1"
#endif

/** @brief Where the command line options are stored */
struct args_info_t
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  int expand_flag;	/**< @brief Expand single-instruction rep/repne loops (default=off).  */
  const char *expand_help; /**< @brief Expand single-instruction rep/repne loops help description.  */
  int bytes_flag;	/**< @brief Log bytecodes (default=off).  */
  const char *bytes_help; /**< @brief Log bytecodes help description.  */
  int memory_flag;	/**< @brief Log memory access (default=off).  */
  const char *memory_help; /**< @brief Log memory access help description.  */
  int quiet_flag;	/**< @brief Do not disassemble instructions (default=on).  */
  const char *quiet_help; /**< @brief Do not disassemble instructions help description.  */
  int general_flag;	/**< @brief Log general registers value (default=off).  */
  const char *general_help; /**< @brief Log general registers value help description.  */
  int hybrid_flag;	/**< @brief Use inlined code to cache registers and make fewer clean calls (default=off).  */
  const char *hybrid_help; /**< @brief Use inlined code to cache registers and make fewer clean calls help description.  */
  int inline_flag;	/**< @brief Use inline code for improved performance (default=off).  */
  const char *inline_help; /**< @brief Use inline code for improved performance help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int expand_given ;	/**< @brief Whether expand was given.  */
  unsigned int bytes_given ;	/**< @brief Whether bytes was given.  */
  unsigned int memory_given ;	/**< @brief Whether memory was given.  */
  unsigned int quiet_given ;	/**< @brief Whether quiet was given.  */
  unsigned int general_given ;	/**< @brief Whether general was given.  */
  unsigned int hybrid_given ;	/**< @brief Whether hybrid was given.  */
  unsigned int inline_given ;	/**< @brief Whether inline was given.  */

} ;

/** @brief The additional parameters to pass to parser functions */
struct args_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure args_info_t (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure args_info_t (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *args_info_t_purpose;
/** @brief the usage string of the program */
extern const char *args_info_t_usage;
/** @brief the description string of the program */
extern const char *args_info_t_description;
/** @brief all the lines making the help output */
extern const char *args_info_t_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int args_parser (int argc, char **argv,
  struct args_info_t *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use args_parser_ext() instead
 */
int args_parser2 (int argc, char **argv,
  struct args_info_t *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int args_parser_ext (int argc, char **argv,
  struct args_info_t *args_info,
  struct args_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int args_parser_dump(FILE *outfile,
  struct args_info_t *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int args_parser_file_save(const char *filename,
  struct args_info_t *args_info);

/**
 * Print the help
 */
void args_parser_print_help(void);
/**
 * Print the version
 */
void args_parser_print_version(void);

/**
 * Initializes all the fields a args_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void args_parser_params_init(struct args_parser_params *params);

/**
 * Allocates dynamically a args_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized args_parser_params structure
 */
struct args_parser_params *args_parser_params_create(void);

/**
 * Initializes the passed args_info_t structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void args_parser_init (struct args_info_t *args_info);
/**
 * Deallocates the string fields of the args_info_t structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void args_parser_free (struct args_info_t *args_info);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int args_parser_required (struct args_info_t *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ARGS_H */