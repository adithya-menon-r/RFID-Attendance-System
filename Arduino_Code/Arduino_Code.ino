// Including all necessary libraries and header fiiles
#include <stdlib.h>
#include <string.h>
#include <SD.h>
#include <SPI.h>
#include <LiquidCrystal.h>

// Initialising all Pin numbers for connected devices
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
const int GREEN_LED = 5;
const int RED_LED = 6;
const int BUTTON = 2;

// Structure definiton for a student
typedef struct {
    char rfid[13];
    char name[31];
    char roll[31];
    int present;
} Student;

Student *students = NULL;

int student_count = 0;
int char_count = 0;
char current_id[13] = "";
bool lastButtonState = LOW; 

File record;
File myFile;

// Function Prototypes
void reset_display();
void read_students();
void read_line(File file, char *buffer, int len);
void process_rfid();
void dump_attendance();

void setup() {
    // Start serial communication at 9600 baud rate - this lets us see debug messages on the computer
    Serial.begin(9600);
    
    // Set up our LED pins as outputs - they'll be used for visual feedback
    pinMode(RED_LED, OUTPUT);   // RED_LED will show errors/failures
    pinMode(GREEN_LED, OUTPUT); // GREEN_LED will show success
    
    // Set up the button with internal pull-up resistor
    // INPUT_PULLUP means the button reads HIGH when not pressed, LOW when pressed
    // This saves us from needing an external resistor
    pinMode(BUTTON, INPUT_PULLUP);

    // Initialize the LCD screen that's 20 characters wide and 4 rows tall
    lcd.begin(20, 4);
    lcd.clear();               // Clear any garbage that might be on the screen
    lcd.print("Initialising..."); // Show initial message
    delay(500);               // Wait half a second so people can read the message

    // Try to initialize the SD card reader
    if (!SD.begin()) {
        // If SD card fails to initialize:
        lcd.clear();
        lcd.setCursor(0, 0);           // Move cursor to top-left corner
        lcd.print("SD Card Failed!");  // Show error message on LCD if SD initialization fails
        digitalWrite(RED_LED, HIGH);    // Turn on red LED to indicate failure
        while (1);                      // Freeze program forever (requires reset to continue)
    }

    // Load all student data from the SD card into memory
    read_students();
    
    // Print all loaded student data to Serial Monitor for debugging
    lcd.clear();
    lcd.print("Details Loaded...");

    // Serial.println("Loaded students:");
    // for (int i = 0; i < student_count; i++) {
    //     Serial.print("RFID: ");
    //     Serial.print(students[i].rfid);
    //     Serial.print(" Name: ");
    //     Serial.println(students[i].name);
    // }
    
    reset_display(); // Show the default screen
    Serial.println("Ready to read RFID tags...");
}

void loop() {
    // Static variables maintain their values between loop iterations
    static String inputString = "";          // Stores the incoming RFID digits
    static bool showingDefault = true;       // Tracks if we're showing the welcome screen
    static unsigned long lastInputTime = 0;  // Tracks when we last received RFID input
    static bool buttonMessageShown = false;  // Prevents button message from repeating
    
    // Read the current state of the button (LOW means pressed, due to INPUT_PULLUP)
    bool currentButtonState = digitalRead(BUTTON);
    
    // Check if button was just pressed (LOW) and was previously not pressed (HIGH)
    // This creates a single trigger when button is pressed instead of continuous
    if (currentButtonState == LOW && lastButtonState == HIGH) {
        // Show that we detected the button press
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Button Pressed");
        lcd.setCursor(0, 1);
        lcd.print("Exporting Attendance");
        delay(1000);  // Display message for 1 second
        
        // Save all attendance data to SD card
        dump_attendance();
        
        buttonMessageShown = true;  // Remember that we showed the message
        delay(200);  // Small delay to prevent button bounce
    }
    lastButtonState = currentButtonState;  // Store button state for next comparison
    
    // Check if there's any RFID data coming in through Serial
    while (Serial.available() > 0) {
        char c = Serial.read();  // Read one character at a time
        
        // If we haven't collected all 12 digits yet
        if (char_count < 12) {
            inputString += c;  // Add the new digit to our string
            char_count++;      // Keep track of how many digits we have
            
            // Update the LCD to show progress
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Reading RFID...");
            lcd.setCursor(0, 1);
            lcd.print("Count: ");
            lcd.print(char_count);
            lcd.print("/12");
            lcd.setCursor(0, 2);
            lcd.print(inputString);  // Show the digits as they come in
            
            showingDefault = false;          // We're not showing welcome screen anymore
            lastInputTime = millis();        // Remember when we got this digit
        }
        
        // If we've collected all 12 digits
        if (char_count == 12) {
            // Print the complete RFID to Serial Monitor for debugging
            Serial.print("Card ID: ");
            Serial.println(inputString);
            
            // Convert the String to a char array (needed for comparison)
            inputString.toCharArray(current_id, sizeof(current_id));
            current_id[12] = '\0';  // Add string terminator
            
            // Process the complete RFID (check if valid, mark attendance, etc)
            process_rfid();
            
            // Reset everything for the next card
            char_count = 0;
            inputString = "";
            showingDefault = true;
            break;  // Exit the while loop
        }
    }
    
    // If we're not showing the welcome screen, no data is coming in,
    // and it's been more than 3 seconds since the last input
    if (!showingDefault && !Serial.available() && (millis() - lastInputTime > 3000)) {
        reset_display();  // Show the default screen again
        showingDefault = true;
    }
}

void reset_display() {
  // Clear any previous content on the LCD screen
    lcd.clear();
    // Set cursor to the first row, first column (top left)
    lcd.setCursor(0, 0);
    lcd.print("  23CSE201 PROJECT  ");
    // Move cursor to the second row and print a separator line
    lcd.setCursor(0, 1);
    lcd.print("--------------------");
    // Set cursor to the third row to show the "Ready for RFID" status message
    lcd.setCursor(0, 2);
    lcd.print("Ready for RFID     ");
    // Move cursor to the fourth row to prompt the user to scan an RFID card
    lcd.setCursor(0, 3);
    lcd.print("Scan your card...  ");
}

void process_rfid() {
    // initialize Flag to track if the scanned RFID matches any stored student
    bool found = false;
    
    //display processing rfid on serial along with inputted rfid.
    Serial.write("Processing RFID: ");
    Serial.println(current_id);

    //loop to compare inputted rfit with stored students by iterating through stored students.
    for (int i = 0; i < student_count; i++) {
        //printing message on serial to show that comparing is happening
        Serial.print("Comparing with stored RFID: ");
        Serial.println(students[i].rfid);

        //checking if the inputted rfid is same as the rfid of the student corresponing to current iteration.
        if (strcmp(current_id, students[i].rfid) == 0) {
            //if true,Match found is printed and found flag set to True
            Serial.println("Match found!");
            found = true;

            //now that rfid is confirmed to be in storage/file,if the student isn't marked present,he/she is marked present
            //by changing the present member of the student to 1.
            if (students[i].present == 0) {
                students[i].present = 1;
                
                // When marked present,the students name and roll number are displayed.along with student present message.
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Student Present:");
                lcd.setCursor(0, 1);
                lcd.print(students[i].name);
                lcd.setCursor(0, 2);
                lcd.print(students[i].roll);

                // when a student is marked present,the green LED glows for 200 ms to indicate success.
                digitalWrite(GREEN_LED, HIGH);
                delay(200);
                digitalWrite(GREEN_LED, LOW);
                delay(500);
            } else {
                // if the student is already marked present i.e present member was already=1, already marked message displayed.
                // on lcd
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Already Marked!");
                lcd.setCursor(0, 1);
                lcd.print(students[i].name);

                // the RED light is ON for 200ms to indicate error.
                digitalWrite(RED_LED, HIGH);
                delay(200);
                digitalWrite(RED_LED, LOW);
                delay(500);
            }
            break;
        }
    }
    //if the flag found is still false,it means that the inputted rfid doesnt exist in the file/storage.
    if (!found) {
        //so "No match found" message is printed in serial and "invalid rfid" on lcd along with the rfid.
        Serial.println("No match found!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Invalid RFID!");
        lcd.setCursor(0, 1);
        lcd.print(current_id);

        //the red light is turned on for 200 ms to indicate an error.
        digitalWrite(RED_LED, HIGH);
        delay(200);
        digitalWrite(RED_LED, LOW);
        delay(500);
    }
    //this returns the lcd to the default screen of accepting rfids.
    reset_display();
}

// Function to read a single line in the data file
void read_line(File file, char *buffer, int len) { 
    int i = 0;  // Current position in buffer
    // Continues reading until there is no more data or the buffer is full
    while (file.available() && i < len - 1) {
        char c = file.read(); // Read single character from file
        
        // Stops reading when it detects newline or carriage return characters
        if (c == '\n' || c == '\r') {
            if (i > 0) break;     // If data was read before ending, end reading process
            else continue;        // If no data was read, empty line is understood and line is skipped
        }
        
        buffer[i++] = c; // Stores the read character and move buffer position by 1
    }
    buffer[i] = '\0'; // Adds null char at the end of the buffer (char array)
}

// Function to open the data file and then load saved data
void read_students() {
    // Opens the students.txt file for reading data
    record = SD.open("students.txt");
    if (record) {
        // record.available) returns the number of bytes in the file from the file pointer
        while (record.available()) {
            char line[100];
            read_line(record, line, sizeof(line)); // Calls read_line function to read a single line to line buffer
            
            // Ignores the read lien if it is less than 5 characters long - malformed data
            if (strlen(line) < 5) continue; 
            
            // Reallocates memory for one more student as a new line (new student) was read
            Student *newArray = (Student *)realloc(students, (student_count + 1) * sizeof(Student));
            // If memory is full, realloc won't happen and newArray will be a NULL pointer
            if (newArray == NULL) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Memory Full!");  // Displays the error in LCD
                free(students);
                while (1); // Infinite Loop due to fatal error
            }
            students = newArray; // Assign reallocated memory block (Student *) to students

            // Initialise temp buffer
            int pos = 0;
            int field = 0;
            char temp[31] = {0};
            int temp_pos = 0;
            
            while (line[pos] != '\0') { // Continue reading till the read char isn't the null char
                if (line[pos] == ',' || line[pos + 1] == '\0') {
                    // If the char is the last char, include it and then proceed to assignment
                    if (line[pos + 1] == '\0' && line[pos] != ',') {
                        temp[temp_pos++] = line[pos];
                    }
                    temp[temp_pos] = '\0'; //Assign null char at the end of the temp buffer
                    
                    // Correctly assign the read data (temp buffer) to the correct member based on value of field
                    switch(field) {
                        case 0: // RFID Member
                            strncpy(students[student_count].rfid, temp, 12);
                            students[student_count].rfid[12] = '\0';
                            break;
                        case 1: // Name member
                            strncpy(students[student_count].name, temp, 30);
                            students[student_count].name[30] = '\0';
                            break;
                        case 2: // Roll Number Member
                            strncpy(students[student_count].roll, temp, 30);
                            students[student_count].roll[30] = '\0';
                            break;
                    }
                    
                    // reset and clear the temp buffer for next field
                    memset(temp, 0, sizeof(temp)); // Sets all characters to 0
                    temp_pos = 0;
                    field++; // Going to next field
                } else {
                    temp[temp_pos++] = line[pos]; // If char is not , or end add it to the temp buffer
                }
                pos++; // Moving the buffer position ahead
            }
            
            students[student_count].present = 0; // Initialises Student to Absent
            student_count++; // Increments the number of students loaded
        }
        record.close(); // Closes the data file after loading data
        
        // Displays successful data loading with green LED
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        delay(200);
        digitalWrite(GREEN_LED, LOW);
    } else {
        lcd.clear();
        lcd.print("Data Loading Error"); // Displays error if File couldn't be opened
        while (1); // Goes into infinite loop because of fatal error
    }
}

// This function writes the details of present students to a text file in virtual SD card connected to Arduino.
void dump_attendance() {
    // Prints message to Serial Monitor to indicate the starting of dump process and 
    //prints the total number of present students.
    Serial.println("Dumping attendance...");
    Serial.print("Total students: ");
    Serial.println(student_count);

    // Opens the file "data.txt" on the SD card in write mode.
    myFile = SD.open("data.txt", FILE_WRITE);

    // Checks if the file was opened successfully
    if (myFile) {
        // Loops through the struct Student array and writes the name and roll number 
        //of each present student as comma separated values to the "data.txt" file. 
        for (int i = 0; i < student_count; i++) {
            if (students[i].present == 1) {
                myFile.print(students[i].name); 
                myFile.print(',');              
                myFile.println(students[i].roll); 
            }
        }
        // Closes the "data.txt" file after writing all present students' data
        myFile.close();

        // Prints completion message to the Serial Monitor
        Serial.println("Done. Attendance Exported");

        // Makes Green LED blink to indicate successful export of attendace 
        digitalWrite(RED_LED, LOW);  // Makes sure Red LED is off before blinking
        digitalWrite(GREEN_LED, HIGH);
        delay(200);  //Green LED blinks for 200ms
        digitalWrite(GREEN_LED, LOW);
    } else {
        // Prints error message to Serial Monitor if file could not be opened.
        Serial.println("Error while writing File");
    }
}
