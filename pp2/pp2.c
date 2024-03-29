// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/pp2.h>

#include <stdint.h>

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/pp2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/pp2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int (pp2_test_packet)(uint32_t cnt) {
  /* To be completed */
  printf("%s(%u): under construction\n", __func__, cnt);

  int ipc_status, r, irq_set = 0;
  message msg;

  bool done = false;

  // XXX : you may need to add some code here

  // interrupt loop
  while (!done) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // subscribed interrupt
                                                   // handle interrupt
          }
          break;
        default:
          break;
      }
    }
  }

  return r;
}

int (pp2_test_legacy)(uint32_t cnt) {
  /* To be completed */
  printf("%s(%u): under construction\n", __func__, cnt);

  return 1;
}
