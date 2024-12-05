# RFID Attendance System

## About
This project uses Arduino and Proteus simulation to create an RFID-based attendance system. RFID tags are simulated via terminal input, with feedback provided through an LCD display and LEDs. Student data is dynamically loaded into memory from a text file, ensuring efficient storage and management. When a valid tag is scanned, the system checks and marks the student as present, avoiding duplicates. When the export button is pressed, the attendance data is exported to a text file on the virtual SD card. This compact solution demonstrates robust hardware-software integration and memory optimization.

This Project was created for the **23CSE201 (Procedural Programming using C)** course.

## Installation and Setup
1. Download the `Source Code` or Clone the repository:

   ```bash
   git clone https://github.com/adithya-menon-r/RFID-Attendance-System.git
   cd RFID-Attendance-System
   ```
   
2. To use the Arduino Mega in Proteus, you need to add the corresponding library file. Download the Simulino Arduino library file from [here](https://github.com/smelpro/Arduino/tree/master/3.Arduino%20en%20Proteus/Simulino%20V3). After downloading the `.LIB` file, add it to your Proteus library folder.

3. Using a tool like WinImage, create a virtual disk image with the file extension `.IMA`.

4. Create a text file named `students.txt` containing student data and save it in the virtual disk image. The data must be in the following format: each line should include the RFID tag value (12 characters), student name, and roll number separated by commas. Example:
    ```
    A1234567890B,John Doe,CS202345  
    C0987654321D,Jane Smith,CS202346  
    E5432109876F,Mark Lee,CS202347  
    B1029384756G,Alice Green,CS202348  
    ```

5. Open the properties of the SD Card component in Proteus and select the path to the virtual disk image.

6. Select the Arduino Mega board in the Arduino IDE and compile the code. After compilation, copy the location of the generated `.hex` file from the terminal.

7. Open the properties of the Arduino Mega in Proteus and add the path to the compiled `.hex` file.

8. Run the Simulation

## Project Structure 
```
RFID-Attendance-System/
 ├── Arduino_Code/                      
 │    └── Arduino_Code.ino               # Main Arduino code
 ├── RFID_Attendance_System.pdsprj       # Proteus simulation Project file
 └── virtual_sd_card.IMA                 # Virtual SD card Disk Image                   
```

## Core Features
### Student Data Management
- Student records are stored in a struct array. Each struct holds student information like RFID, name, roll number and present status. Upon startup, the system loads these records from `students.txt` in the virtual SD card. The array is dynamically allocated using `malloc`, and when new students are added, `realloc` is used to expand the array's memory allocation to accommodate additional records. This ensures efficient memory usage as the number of students can vary. 

### RFID Tag Handling
- Once the student data is loaded into memory, the terminal is initialised (used to simulate an RFID scanner) and listens for tag input. Upon scanning, the system checks if the tag matches any student records. If the tag is valid, the student is marked present and the student details are displayed on the LCD. Invalid tags trigger an error message on the LCD, and the red LED lights up. 

### Attendance Marking
- When a valid RFID tag is scanned, the system verifies whether the student has already been marked present to prevent duplicate entries. If not, it updates the student's attendance status in the `Student` struct and lights up a green LED to indicate success. If the student is already marked present, an error message is displayed on the LCD, and the red LED lights up.

### Attendance Log Export
- When the export button is pressed, the system iterates through the `students` struct array and writes the details of students marked as present (status set to 1) to a text file (`data.txt`) on the virtual SD card. After successfully completing the export, a confirmation message is displayed on the LCD.

## License
This project is licensed under the [MIT LICENSE](LICENSE).

## Developers
- [Adithya Menon R](https://www.linkedin.com/in/adithya-menon-r)
- [Narain BK](https://www.linkedin.com/in/narain-bk)
- [PG Karthikeyan](https://www.linkedin.com/in/karthikeyan-pg-95a5b6291)