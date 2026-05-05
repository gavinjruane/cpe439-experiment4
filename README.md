# Experiment 4

Gavin Ruane &bullet; CPE 439-01 &bullet; 6 May 2026

## Demonstration Video

My demonstration video is available on [YouTube](...) (if you would like to review it).

## Questions

1. List the two advantages that event groups have over using semaphores for inter-process signaling.

	While both semaphores and event groups have important uses, event groups are often superior for *signaling multiple events* as their name implies and *signaling events to multiple tasks*.

2. Although event groups can handles multiple events, list the two main advantages that using event groups has over using a single semaphore.

	Event groups use less resources than a single semaphore.

3. Briefly describe the two sets of event group functionality associated with the FreeRTOS API ~~`xEventGroupSyncFromISR()`~~ `xEventGroupSync()`.

	The function `xEventGroupSyncFromISR()` does not exist, so I will answer this question for `xEventGroupSync()`. 

4. Briefly describe how event groups can be configured to use either OR or AND functions when waiting for event flags to be set in the event register.

	Using an **AND** operation with an event group means that all bits specified in the `uxBitsToWaitFor` field must be set before the function will return and unblock. Using an **OR** operation with an event group means that at least one bit must be set for the function to unblock, but not all bits are necessary.

	As an example, consider the wait bits `0b0011`. If we were using an **AND** operation, we would need `0b0011` to be satisfied before the function would unblock. If we were using an **OR** operation, we would only need at least `0b0001` or `0b0010` to be set before continuing. (`0b0011` would be acceptable as well.)

5. Briefly describe how tasks waiting on certain event groups specify which event groups they are waiting for.

	Each event group has its own handle that can be used by the event group API functions to specify which group they should operate on. For many systems, it might be useful to declare event group handles as global variables so that all tasks can access the handle and either set or wait for bits in the group; however, other systems might prefer to pass the handle as an argument to avoid other tasks changing an event group to which they should not have access.

	Once a task has the event group handle, it can call functions like `xEventGroupSetBits()` to update the event group or `xEventGroupWaitBits()` to check the status of the event group.

8. Briefly describe how tasks can choose to handle what happened to event flags after the task awakens to some set of event flags in the event register being set.

	Using the `xEventGroupWaitBits()` function, you can state whether you would like the bits specified in the `uxBitsToWaitFor` field to be cleared on the return of the function. (This does *not* apply to when the function returns because of a timeout.) If this value is set to true, the bits specified in `uxBitsToWaitFor` will be cleared once this function unblocks.

	This functionality could be useful if you want only one task to be able to run once the bits are set, requiring other tasks dependent on the event group to wait until the bits are set again. It is also useful for ensuring that future updates to the event group are not changed by a stale event group.

## System Design & Explanation

### Four Tasks

#### Default Task (Pre-Event Group Flow)
In my implementation, the default task creates the event group and launches the four tasks needed to perform this experiment. Once it completes these tasks, it enters an infinite loop.

#### Task 1
Task 1 will immediately delay for 300 milliseconds and then set the first bit in the event group, leaving the event group to have a value of `0x01`.

#### Task 2
Task 2 immediately blocks waiting for Task 1 to set its bit. Once Task 1 sets the first bit, it unblocks and sets the *second* bit in the event group, leaving the event group to have a value of `0x03`.

#### Task 3
Task 3 immediately blocks waiting for Task 2 to set its bit. Once Task 2 sets the second bit, it unblocks and enters a spin poll loop waiting for a button press. Once it detects a button press, Task 3 sets the *third* bit in the event group, leaving the event group to have a value of `0x07`.

#### Task 4
Task 4 immediately blocks waiting for the bits set by Task 1, Task 2, and Task 3. Once *all* three previous tasks set their respective bits, Task 4 unblocks and displays a simple blinking pattern on the onboard LED.

### Problems

During lab time on Monday, May 4, I had mostly figured out a solution to the experiment, but I was having trouble getting my fourth task to run at all. In my design, I launch the four tasks needed for this experiment from the default task, and whichever task I scheduled last (or fourth) would *never* run. This did not have anything to do with blocking for bits in the event group.

I briefly searched online why one task might never run in FreeRTOS, and I came across a forum post that suggested there might be some memory issues. I reduced the size of each task's stack to half of its default value, and that solved the issue!

I am still confused as to why this issue would occur, especially because I really did not use much memory in this experiment. I believe that I have used more memory in bare-metal projects in the past, so my assumption would be that there is maybe a setting in FreeRTOS that I need to adjust. For reference, I am using the recommended NUCLEO board for this class (NUCLEO-L476RG), so I do not believe that I have constrained hardware or anything like that. Still, I am not satisfied with this solution! 

## Program Code

I did not include all of the code generated by STM32CubeMX nor some of the boilerplate to focus on the code I actually wrote for this experiment.

```C
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include <stdbool.h>

/* Constants */
#define DELAY1_MS 				300
#define TASK1_BITS				(1 << 0)
#define TASK2_BITS				(1 << 1)
#define TASK3_BITS				(1 << 2)
#define STACK_SIZE				128 * 2

/* Macro "functions" */
#define PA5_on() 				GPIOA->BSRR = (1 << 5);
#define PA5_off() 				GPIOA->BRR = (1 << 5);
#define LED_status_toggle() 	led_status = !led_status;
#define PC13_is_on() 			(GPIOC->IDR & GPIO_PIN_13) == 0
#define PA5_handle() \
	if ( led_status ) { \
		PA5_on(); \
	} else { \
		PA5_off(); \
	}

/* Task handle definitions */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
	.name = "defaultTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t firstTaskHandle;
const osThreadAttr_t firstTask_attributes = {
	.name = "firstTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t secondTaskHandle;
const osThreadAttr_t secondTask_attributes = {
	.name = "secondTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t thirdTaskHandle;
const osThreadAttr_t thirdTask_attributes = {
	.name = "thirdTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t fourthTaskHandle;
const osThreadAttr_t fourthTask_attributes = {
	.name = "fourthTask",
	.stack_size = STACK_SIZE,
	.priority = (osPriority_t) osPriorityNormal
};

/* Global variables */
EventGroupHandle_t group;
bool led_status = false;

/* STM32 function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* Task function prototypes */
void StartDefaultTask(void *argument);
void FirstTask (void *argument);
void SecondTask (void *argument);
void ThirdTask (void *argument);
void FourthTask (void *argument);

/* Function prototypes */
void debug_blink (int amount);

/* Main function 
   -> Initializes core functionality and kernel */
int main(void) {
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();

	osKernelInitialize();
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	osKernelStart();

	while (1);
}

/* debug_blink function
	-> Given an amount, blinks the LED amount number of times */
void debug_blink (int amount) {
	for ( int i = 0; i < amount * 2; i++ ) {
		LED_status_toggle();
		PA5_handle();
		vTaskDelay(amount * 10);
	}
}

/* StartDefaultTask function
	-> Creates the event group and launches all four tasks */
void StartDefaultTask (void *argument) {
	group = xEventGroupCreate();

	firstTaskHandle = osThreadNew(FirstTask, NULL, &firstTask_attributes);

	secondTaskHandle = osThreadNew(SecondTask, NULL, &secondTask_attributes);

	thirdTaskHandle = osThreadNew(ThirdTask, NULL, &thirdTask_attributes);

	fourthTaskHandle = osThreadNew(FourthTask, NULL, &fourthTask_attributes);

	while (1);
}

/* FirstTask function
	-> Delays for 300ms and then sets a bit in the event group */
void FirstTask (void *argument) {
	vTaskDelay(DELAY1_MS);

	xEventGroupSetBits(group, TASK1_BITS);

	while (1);
}

/* SecondTask function
	-> Waits for FirstTask's bits and then sets a bit in the event group */
void SecondTask (void *argument) {
	xEventGroupWaitBits(
		group,
		TASK1_BITS,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	xEventGroupSetBits(group, TASK2_BITS);

	while (1);
}

/* ThirdTask function
	-> Waits for SecondTask's bits and polls for a button press; 
	   when the button press arrives, sets a bit in the event group */
void ThirdTask (void *argument) {
	xEventGroupWaitBits(
		group,
		TASK1_BITS | TASK2_BITS,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	while ( ! PC13_is_on() );

	xEventGroupSetBits(group, TASK3_BITS);

	while (1);
}

/* FourthTask function
	-> Waits for the bits set byFirstTask, SecondTask, and ThirdTask
	   and then displays a blinking pattern */
void FourthTask (void *argument) {
	xEventGroupWaitBits(
		group,
		TASK1_BITS | TASK2_BITS | TASK3_BITS,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	while (1) {
		debug_blink(25);
	}

	while (1);
}
```
