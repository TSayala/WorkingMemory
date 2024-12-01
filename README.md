# Arduino-Based Working Memory Test Interface

A portable, self-contained working memory testing apparatus designed for laboratory mice, with adaptable difficulty levels suitable for both simple animal models and human subjects.

## Overview

This project implements a working memory task using an Arduino-driven LCD touchscreen interface. The system presents visual cues and tests subjects' ability to recall and respond to them, offering both synchronous and asynchronous testing modes.

### Key Features

- Fully portable, battery-powered design
- On-board data storage for performance metrics
- Scalable difficulty levels
- Touch-based interface
- Two distinct testing modes:
  - Synchronous: simultaneous display of cues and directional arrows
  - Asynchronous: sequential display of cues followed by directional arrows

## Hardware Requirements

- Arduino Due
- 7-inch capacitive touch screen shield with SSD1963 controller
- Compatible shield
- Wireless, battery-powered chassis

## Software Architecture

The program operates through several distinct states:

- `menu_screen`: Task selection interface
- `wait_for_start`: Experiment preparation and initialization
- `select_trial`: Trial randomization and setup
- `draw_stuff`: Visual element rendering
- `show_results`: Performance metrics display

### Task Types

#### Synchronous Mode
- Displays cues and directional arrows simultaneously
- Tests immediate recall ability
- Selection screen follows for response

<img src="https://github.com/user-attachments/assets/ad3f2085-d607-4c0a-a13c-43813c6283c4" width="400" alt="Synchronous Experiment Flow">

*Synchronous test mode showing simultaneous cue and arrow presentation*

<img src="https://github.com/user-attachments/assets/eeb990e6-0df8-46f3-964f-9706a21d1206" width="400" alt="Selection Screen">

*Selection interface where subjects choose the correct cue*

#### Asynchronous Mode
- Displays cues first, followed by directional arrows
- Tests working memory retention
- Selection screen follows for response

<img src="https://github.com/user-attachments/assets/b0f49c7f-7066-44a6-ba1d-d4d2e88abb81" width="400" alt="Initial Cue Display">

*First phase: Display of initial cues without arrow presentation*

<img src="https://github.com/user-attachments/assets/faf97893-8d84-4586-91b1-016f0588863e" width="400" alt="Arrow Display">

*Second phase: Display of directional arrows generated after the cues disappear*

## Performance Tracking

The system records:
- Selection latency
- Success percentage
- Overall performance metrics

## Future Development

Planned enhancements include:
- Enhanced graphical user interface
- Expanded task complexity options
- Advanced data collection and analysis features
- Permanent chassis design

## Project Constraints

This project was developed under the following constraints:
- Limited C++ knowledge
- Arduino platform requirements
- Budget restrictions
- 2-month development timeframe
- Limited graphics library availability
