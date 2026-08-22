// Unit tests for boolean/level parsing and log output.
#include "log.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Swap stderr for the given fd; returns the saved fd or -1.
static int capture_start(int out_fd) {
  fflush(stderr);
  int saved = dup(2);
  if (saved < 0 || dup2(out_fd, 2) < 0) {
    return -1;
  }
  return saved;
}

static void capture_stop(int saved) {
  fflush(stderr);
  dup2(saved, 2);
  close(saved);
}

int main(void) {
  // parse_bool: true/false spellings, case-insensitive.
  T_CHECK(parse_bool("yes") == 1);
  T_CHECK(parse_bool("YES") == 1);
  T_CHECK(parse_bool("on") == 1);
  T_CHECK(parse_bool("true") == 1);
  T_CHECK(parse_bool("True") == 1);
  T_CHECK(parse_bool("1") == 1);
  T_CHECK(parse_bool("no") == 0);
  T_CHECK(parse_bool("No") == 0);
  T_CHECK(parse_bool("off") == 0);
  T_CHECK(parse_bool("false") == 0);
  T_CHECK(parse_bool("0") == 0);
  T_CHECK(parse_bool("maybe") == -1);
  T_CHECK(parse_bool("2") == -1);
  T_CHECK(parse_bool("") == -1);
  T_CHECK(parse_bool(NULL) == -1);

  // parse_log_level: names and numbers.
  T_CHECK(parse_log_level("off") == LOG_OFF);
  T_CHECK(parse_log_level("none") == LOG_OFF);
  T_CHECK(parse_log_level("fatal") == LOG_FATAL);
  T_CHECK(parse_log_level("error") == LOG_ERROR);
  T_CHECK(parse_log_level("err") == LOG_ERROR);
  T_CHECK(parse_log_level("warn") == LOG_WARN);
  T_CHECK(parse_log_level("warning") == LOG_WARN);
  T_CHECK(parse_log_level("info") == LOG_INFO);
  T_CHECK(parse_log_level("DEBUG") == LOG_DEBUG);
  for (int i = 0; i <= 5; i++) {
    const char d[2] = {'0' + i, 0};
    T_CHECK(parse_log_level(d) == i);
  }
  T_CHECK(parse_log_level("6") == -1);
  T_CHECK(parse_log_level("-1") == -1);
  T_CHECK(parse_log_level("bogus") == -1);
  T_CHECK(parse_log_level("") == -1);
  T_CHECK(parse_log_level(NULL) == -1);

  // log_msg: level filtering and "prog: level: message" format.
  char tmpl[] = "/tmp/machash_log.XXXXXX";
  int fd = mkstemp(tmpl);
  T_CHECK(fd >= 0);
  if (fd >= 0) {
    log_level = LOG_WARN;
    int saved = capture_start(fd);
    T_CHECK(saved >= 0);
    if (saved >= 0) {
      log_msg("t", LOG_DEBUG, "hidden %d", 1);
      log_msg("t", LOG_INFO, "hidden %d", 2);
      log_msg("t", LOG_WARN, "warnline %d", 42);
      log_msg("t", LOG_ERROR, "errline");
      log_msg("t", LOG_FATAL, "fatalline");
      capture_stop(saved);
    }
    lseek(fd, 0, SEEK_SET);
    char buf[512] = {0};
    ssize_t r = read(fd, buf, sizeof(buf) - 1);
    T_CHECK(r > 0);
    T_CHECK(strstr(buf, "hidden") == NULL);
    T_CHECK(strstr(buf, "t: warning: warnline 42") != NULL);
    T_CHECK(strstr(buf, "t: error: errline") != NULL);
    T_CHECK(strstr(buf, "t: fatal: fatalline") != NULL);

    // LOG_OFF suppresses even fatal messages.
    log_level = LOG_OFF;
    lseek(fd, 0, SEEK_SET);
    ftruncate(fd, 0);
    saved = capture_start(fd);
    if (saved >= 0) {
      log_msg("t", LOG_FATAL, "nope");
      capture_stop(saved);
    }
    lseek(fd, 0, SEEK_SET);
    memset(buf, 0, sizeof(buf));
    r = read(fd, buf, sizeof(buf) - 1);
    T_CHECK(r == 0);

    // log_abort exits with status 1 and leaves a fatal message.
    log_level = LOG_FATAL;
    lseek(fd, 0, SEEK_SET);
    ftruncate(fd, 0);
    pid_t pid = fork();
    T_CHECK(pid >= 0);
    if (pid == 0) {
      int s = capture_start(fd);
      if (s < 0) {
        _exit(98);
      }
      log_abort("t", "boom %s", "now");  // exits with status 1
    }
    int st = 0;
    waitpid(pid, &st, 0);
    T_CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 1);
    lseek(fd, 0, SEEK_SET);
    memset(buf, 0, sizeof(buf));
    r = read(fd, buf, sizeof(buf) - 1);
    T_CHECK(r > 0);
    T_CHECK(strstr(buf, "t: fatal: boom now") != NULL);

    close(fd);
    remove(tmpl);
  }

  T_SUMMARY("unit_log");
}
