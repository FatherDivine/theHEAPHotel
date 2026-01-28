/* 
* Coded by FatherDivine in nano then vim (parrotOS) once I realized vim has C syntax.
* For educational purposes only, hence heavy comments to educate.
* For the naabs: compile in linux via cli: "cc theHEAPHotel.c" -o theHEAPHotel && execute ./theHEAPHotel

* WIP: Add saving checkins/out to register file (fopen saved to hard disk, encrypted phase 2 if possible) to be used by a 
* "management menu item" (encrypted password) that allows viewing the registery checkins/out as if you are hotel management. 
* Eventually add cost of room + tax tied to random generator that generates between $50 a night float difference depending
* if an event in town, slow night etc.
*/
#include <limits.h> // for PATH_MAX
#include <stdint.h> // for uint8_t
#include <stdio.h>	// printf, scanf, getline
#include <stdlib.h> // for HEAP, free, malloc, realloc
#include <string.h> // for strcmp() string compare
#include <unistd.h> // for getcwd

// 63 max rooms when total_books == 6
// Math: 2^n -1
// 2^6 = 64
// 64-1 = 63 (given the terminating char)
#define MAX_ROOMS 		63
#define REAL_PASSWORD	"DRitchie1941"

// BIT-FIELDS: We are manually defining exactly how many bits each variable uses.
// 'menu_choice : 3' means it uses only 3 bits (binary 111 = 7).
// If we tried to store the number 8 (binary 1000), it would overflow!
// '__attribute__((packed))' tells the compiler not to add empty "padding" bytes between variables.
struct GuestStatus {
	uint8_t is_authorized	: 1; // 0 or 1
	uint8_t is_cleared		: 1; // 0 or 1
	uint8_t total_books		: 6; // 6 bits == 63 rooms total
} __attribute__((packed));

// Array of 5 "room keys"
char *rooms[MAX_ROOMS] = {NULL};
char reg_buffer[50] = {0};

//Function prototypes go after structs
//These are previews for the compiler to not complain
void management(struct GuestStatus *status, char *reg_buffer);
void readr(FILE *input, FILE *output);
void registryr(struct GuestStatus *status);
void registrye(char *reg_buffer);
void registrycheckin(int room_num, char *guest_name); // for writing to the ledger/register
void registrycheckout(int room_num);

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

	Also clear buffer before getline to ensure it doesn't catch a leftover '\n'
    while (getchar() != '\n' && getchar() != EOF); // Only use if a scanf was right before this
*/


	if (getline(&rooms[room_found], &size, stdin) != -1) {           
        // REMOVE the newline character getline adds at the end
        rooms[room_found][strcspn(rooms[room_found], "\n")] = 0;	

		// Update our bit-field stats
		status->total_books++; 		// Add a booking on using pointer
		status->is_cleared = 0; 	// They are checked in (not cleared)
		
		// Write the full string to the registery
		registrycheckin(room_found +1, rooms[room_found]);
		printf("Success! Room %d booked for %s.\n", room_found + 1, rooms[room_found]);
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
		printf("Evicting %s from Room %d...\n", &rooms[i] ,roomNum);

		// Write the full string to the registery
		registrycheckout(roomNum);

		printf("Room %d is now vacant.\n", roomNum);
	} else{
		printf("Invalid room or room already empty.\n");
	}
}

void management(struct GuestStatus *status, char *reg_buffer){
	char password[21];

	printf("\n--- The HEAP Hotel Management Backend	---\n");
	printf("\nFor authorized users only.\n");
	
	while (status->is_authorized != 1){
		printf("\nPlease enter your password.\n");

		if (scanf("%20s", password) != 1) break; // No '&' needed for arrays; %20s prevents overflow
	
		// CLEAN the buffer after scanf every time
        while (getchar() != '\n' && getchar() != EOF);

		if (strcmp(password, REAL_PASSWORD) != 0) {		

			printf("Wrong Password! Incorrect attempt recorded!");
			
			//open register and write failed attempt to register

			password[0] = '\0';
			//Clear the input buffer so the next loop is fresh
			while (getchar() != '\n' && getchar() != EOF);		
		}else{
			printf("\nPassword accepted!\n");
			status->is_authorized = 1;
		}
	}
		printf("Welcome to the Registry!");
		// Management menu choice
		uint8_t mchoice = 0;

			// The Menu Loop
		while (1){
			printf("\nMenu:\n1:Print Registery\n2:Edit Registry entry\n3:Exit to main menu\nChoice: ");

			// Read user choice
			if (scanf("%d", &mchoice) != 1) break;

			// Clear the buffer so getline doesn't skip
			while (getchar() != '\n');

			if (mchoice == 1){
				registryr(status);
			} else if (mchoice == 2){
				registrye(reg_buffer);
			} else if (mchoice == 3){
				break;
			} else{
				printf("Invalid choice!\n");
			}
	}
}

void mass_checkout(struct GuestStatus *status){
	printf("\n--- Closing Hotel: Final Memory Sweep ---\n");
	for (int i = 0; i < MAX_ROOMS; i++){
		if (rooms[i] != NULL){
			printf("Evicting and freeing Room %d: %s\n", i + 1, rooms[i]);
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

void readr(FILE *input, FILE *output) {
    int c;
    // fgetc reads one character at a time until End Of File (EOF)
    while ((c = fgetc(input)) != EOF) {
        fputc(c, output); // Put the character into the output stream (stdout)
    }
}

// Reads the registery
void registryr(struct GuestStatus *status){

	FILE *file_ptr = fopen("register.txt", "r");

	if (file_ptr == NULL) {
        printf("Error: Could not open registry file for reading.\n");
        return;
    }

	printf("\n--- HOTEL REGISTRY LEDGER ---\n");    // Code to read the file would go here
	
	readr(file_ptr, stdout);
	
	fclose(file_ptr);

	printf("--- End of Registry ---\n");
}

// Edits the registery
void registrye(char *reg_buffer){

	// WILL cat ledger (w/ # to the left first) then ask user which 
	// line item/entry they want to edit.
	printf("Editing entry: %s\n", reg_buffer);

	return;	
}

// Writes a check-in statement to the registery
void registrycheckin(int room_num, char *guest_name){
	FILE *file_ptr = fopen("register.txt", "a");

	//To simplify below, could use: if (file_ptr == NULL) return;
	if (file_ptr == NULL) {
		printf("Error opening file!\n");
		return;
	}
	fprintf(file_ptr, "CHECK-IN: Room %d - Guest: %s\n", room_num, guest_name);
	
	fclose(file_ptr);

	return;
}

// Writes a check-out statement to the registery
void registrycheckout(int room_num){
	FILE *file_ptr = fopen("register.txt", "a");

	//To simplify below, could use: if (file_ptr == NULL) return;
	if (file_ptr == NULL) {
		printf("Error opening file!\n");
		return;
	}
	fprintf(file_ptr, "CHECK-OUT: Room %d\n", room_num);
	
	fclose(file_ptr);

	return;
}

void viewbookings(struct GuestStatus *status){
	printf("\n--- Current Hotel Guests (%u active) ---\n", status->total_books);
	int occupied = 0;
	for (int i = 0; i < MAX_ROOMS; i++){
		if (rooms[i] != NULL){
			printf("Room %d: %s\n", i + 1, rooms[i]);
			occupied = 1;
		}
	}
	if (!occupied) printf("The hotel is currently empty.\n");
}


int main(){
	// Initialize struct(s) & variablesfor use
	struct GuestStatus hotelState = {0, 1, 0};	
	int choice = 0;

	printf("--- Welcome to the HEAP HOTEL ---\n");

	// The Menu Loop
	while (1){
		printf("\nMenu:\n1:Check-in\n2:Check-out\n3:View Bookings\n4:Management Backend\n5:Exit\nChoice: ");

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
			// for else-if that leads to management menu)
			management(&hotelState, reg_buffer);
		} else if (choice == 5){
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