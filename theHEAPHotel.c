/* 
 * Coded by FatherDivine in nano then vim (parrotOS) once I realized vim has C syntax.
 * For educational purposes only, hence heavy comments to educate.
 * For the naabs: compile in linux via cli: "gcc theHEAPHotel.c" && execute a.out
*/
#include <stdio.h>	// printf, scanf, getline
#include <stdint.h> // for uint8_t
#include <stdlib.h> // for HEAP, free, malloc, realloc
#include <unistd.h> // for getcwd
#include <limits.h> // for PATH_MAX

#define MAX_ROOMS 5

// BIT-FIELDS: We are manually defining exactly how many bits each variable uses.
// 'menu_choice : 3' means it uses only 3 bits (binary 111 = 7).
// If we tried to store the number 8 (binary 1000), it would overflow!
// '__attribute__((packed))' tells the compiler not to add empty "padding" bytes between variables.
struct GuestStatus {
	uint8_t menu_choice		: 3; // Max 7
	uint8_t is_cleared		: 1; // 0 or 1
	uint8_t total_books		: 4; // Max 15
} __attribute__((packed));

// Array of 5 "room keys"
char *rooms[MAX_ROOMS] = {NULL};


void booking(struct GuestStatus *status){
/* 
	* 'size' tracks the capacity of the buffer. 
    * Setting it to 0 tells getline: "I have no memory allocated yet, please malloc it for me."
*/
	size_t size = 0;

	int room_found = -1;

	// Find the first empty room (NULL)
	for (int i = 0; i < MAX_ROOMS; i++){
		if (rooms[i] == NULL) {
			room_found = i;
			break;
		}
	}
	
	if (room_found == -1){
		printf("\n[Hotel Full] No rooms available!\n");
		return;
	}

	printf("\nEnter guest name for Room #%d: ", room_found + 1);
/* 
	* KEY CONCEPT: We pass the ADDRESS of the pointer (&rooms[...]).
    * This allows getline to modify the pointer inside our array to point 
    * to the new memory it allocates.
*/
	if (getline(&rooms[room_found], &size, stdin) != -1){			
	
		// Update our bit-field stats
		status->total_books++; 		// Add a booking on using pointer
		status->is_cleared = 0; 	// They are checked in (not cleared)
		
		printf("Success! Room %d booked.\n", room_found + 1);
	}
}


void checkout(struct GuestStatus *status){
	int roomNum;
	printf("\nEnter Room Number to checkout (1-%d): ", MAX_ROOMS);
	scanf("%d", &roomNum);

	// Clearing the buffer the 'C' way
	// SCUBBER: scanf leaves the 'Enter' key (\n) in the input stream.
	// This loop reads and discards characters until the newline is gone.
	// Without this, the next 'getline' would instantly read that leftover newline and skip prompting the user.
	while (getchar() != '\n' && getchar() != EOF);
	
	int i = roomNum - 1;	// Adjust for 0-based array

	if (i >= 0 && i < MAX_ROOMS && rooms[i] != NULL){
		free(rooms[i]);
		rooms[i] = NULL;
		status->total_books--;
		if(status->total_books == 0) status->is_cleared = 1;
		printf("Room %d is now vacant.\n", roomNum);
	} else{
		printf("Invalid room or room already empty.\n");
	}
}


void mass_checkout(struct GuestStatus *status){
	printf("\n--- Closing Hotel: Final Memory Sweep ---\n");
	for (int i = 0; i < MAX_ROOMS; i++){
		if (rooms[i] != NULL){
			printf("Evicting and freeing Room %d: %s", i + 1, rooms[i]);
			free(rooms[i]);
			rooms[i] = NULL;
			// CRITICAL STEP: We manually set the pointer to NULL immediately after freeing.
			// This prevents "Double Free" errors (crashing by freeing twice) 
			// and ensures our "if (rooms[i] == NULL)" checks works correctly later.
		}
	}
	status->total_books = 0;
	status->is_cleared = 1;
	printf("All memory cleared. Hotel is empty.\n");
}


void viewbookings(struct GuestStatus *status){
	printf("\n--- Current Hotel Guests (%u active) ---\n", status->total_books);
	int occupied = 0;
	for (int i = 0; i < MAX_ROOMS; i++){
		if (rooms[i] != NULL){
			printf("Room %d: %s", i + 1, rooms[i]);
			occupied = 1;
		}
	}
	if (!occupied) printf("The hotel is currently empty.\n");
}


int main(){
	struct GuestStatus hotelState = {0, 1, 0};	// Start cleared
	int choice = 0;

	printf("--- Welcome to the HEAP HOTEL ---\n");

	// The Menu Loop
	while (1){
		printf("\nMenu:\n1:Check-in\n2:Check-out\n3:View Bokings\n4:Exit\nChoice: ");

		// Read user choice
		if (scanf("%d", &choice) != 1) break;

		// Clear the buffer so getline doesn't skip
		while (getchar() != '\n');

		if (choice == 1){
			booking(&hotelState);
		} else if (choice == 2){
			checkout(&hotelState);
		} else if (choice == 3){
			viewbookings(&hotelState);
		} else if (choice == 4){
			// Final cleanup before exiting
			mass_checkout(&hotelState);
			printf("Exiting program...\n");
			break;
		} else{
			printf("Invalid choice!\n");
		}

	}
		return 0;
}