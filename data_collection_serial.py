# from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
# import time
# import csv
# from datetime import datetime
# import numpy as np
# from scipy.signal import butter, filtfilt, welch
# import matplotlib.pyplot as plt
# from collections import deque
# import threading

# # Add these global variables at the start of the file
# MAX_POINTS = 50  # Number of points to display in the plot
# agr_history = [deque(maxlen=MAX_POINTS) for _ in range(7)]  # 6 channels + 1 average
# plot_lock = threading.Lock()

# def high_pass_filter(data, cutoff=0.5, fs=250, order=5):
#     """
#     Apply a high-pass filter to the data.
#     :param data: Input EEG data (channels x samples).
#     :param cutoff: Cutoff frequency in Hz.
#     :param fs: Sampling rate in Hz.
#     :param order: Order of the filter.
#     :return: Filtered data.
#     """
#     nyquist = 0.5 * fs
#     normal_cutoff = cutoff / nyquist
#     b, a = butter(order, normal_cutoff, btype='high', analog=False)
#     return filtfilt(b, a, data, axis=1)

# def compute_agr(data, fs=250):
#     """
#     Compute the Alpha-Gamma Ratio (AGR) for EEG data.
#     :param data: Input EEG data (channels x samples).
#     :param fs: Sampling rate in Hz.
#     :return: AGR values for each channel and average AGR across channels.
#     """
#     agr_values = []
    
#     for channel_data in data:
#         # Compute power spectral density using Welch's method
#         freqs, psd = welch(channel_data, fs=fs, nperseg=fs*2)

#         # Extract alpha (8-12 Hz) and gamma (30-100 Hz) power bands
#         alpha_power = np.sum(psd[(freqs >= 8) & (freqs <= 12)])
#         gamma_power = np.sum(psd[(freqs >= 30) & (freqs <= 50)])
        
#         # Compute AGR for the channel
#         agr = alpha_power / gamma_power if gamma_power > 0 else 0
#         agr_values.append(agr)
    
#     # Compute average AGR across channels
#     avg_agr = np.mean(agr_values)
    
#     return agr_values, avg_agr

# def update_plot():
#     """
#     Function to update the plot in real-time
#     """
#     plt.ion()  # Enable interactive mode
#     fig, ax = plt.subplots(figsize=(10, 6))
#     lines = []
    
#     # Create lines for each channel and average
#     colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
#     labels = [f'Channel {i+1}' for i in range(6)] + ['Average']
    
#     for i in range(7):
#         line, = ax.plot([], [], color=colors[i], label=labels[i])
#         lines.append(line)
    
#     ax.set_title('Real-time Alpha-Gamma Ratio (AGR)')
#     ax.set_xlabel('Time Points')
#     ax.set_ylabel('AGR Value')
#     ax.legend()
#     ax.grid(True)
    
#     while True:
#         with plot_lock:
#             # Update each line
#             for i, line in enumerate(lines):
#                 line.set_data(range(len(agr_history[i])), list(agr_history[i]))
            
#             # Adjust y-axis limits
#             all_values = [val for history in agr_history for val in history]
#             if all_values:
#                 ax.set_ylim(min(0, min(all_values)), max(all_values) * 1.1)
#             ax.set_xlim(0, MAX_POINTS)
            
#         plt.draw()
#         plt.pause(0.1)

# def main():
#     # Get start time for filename
#     start_time = datetime.now().strftime("%Y%m%d_%H%M%S")
    
#     params = BrainFlowInputParams()
#     params.serial_port = "COM10"
#     board = BoardShim(BoardIds.CYTON_BOARD, params)
    
#     board.prepare_session()
#     board.start_stream()

#     # Start the plotting thread
#     plot_thread = threading.Thread(target=update_plot, daemon=True)
#     plot_thread.start()

#     while True:
#         time.sleep(5)  # Collect 5 seconds of data (1250 samples at 250 Hz)
#         data = board.get_board_data()  # Get all data and remove it from internal buffer
        
#         print(data.shape)
        
#         # Transpose the data and select relevant channels (e.g., first six EEG channels)
#         transposed_data = data.T
#         selected_data = transposed_data[:, 1:7]  # Assuming columns 1-6 correspond to EEG channels
        
#         # Apply high-pass filter to the selected data
#         filtered_data = high_pass_filter(selected_data.T).T
        
#         # Compute AGR for filtered data
#         agr_values, avg_agr = compute_agr(filtered_data.T)
        
#         # Update the AGR history with plot_lock
#         with plot_lock:
#             for i, value in enumerate(agr_values):
#                 agr_history[i].append(value)
#             agr_history[-1].append(avg_agr)  # Add average AGR
        
#         print(f"AGR values per channel: {agr_values}")
#         print(f"Average AGR across channels: {avg_agr}")
        
#         # Save filtered data and AGR values to CSV file
#         filename = f'brainflow_data_{start_time}.csv'
#         with open(filename, 'a', newline='') as f:
#             writer = csv.writer(f)
#             writer.writerows(filtered_data)  # Save filtered EEG data
            
#             # Save AGR values and average AGR as a separate row
#             writer.writerow(['AGR'] + agr_values + [avg_agr])
        
#     board.stop_stream()
#     board.release_session()

# if __name__ == "__main__":
#     main()

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
import time
import csv
from datetime import datetime
import numpy as np
from scipy.signal import butter, filtfilt, welch
import serial

def high_pass_filter(data, cutoff=0.5, fs=250, order=5):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = butter(order, normal_cutoff, btype='high', analog=False)
    return filtfilt(b, a, data, axis=1)

def compute_agr(data, fs=250):
    agr_values = []
    for channel_data in data:
        freqs, psd = welch(channel_data, fs=fs, nperseg=fs*2)
        alpha_power = np.sum(psd[(freqs >= 8) & (freqs <= 12)])
        gamma_power = np.sum(psd[(freqs >= 30) & (freqs <= 100)])
        agr = alpha_power / gamma_power if gamma_power > 0 else 0
        agr_values.append(agr)
    avg_agr = np.mean(agr_values)
    return agr_values, avg_agr

def map_agr_to_range(agr_value):
    if agr_value > 0.002:
        agr_value = 0.002
    mapped_value = int((agr_value / 0.002) * (200 - 100) + 100)  # Map to range 100-200
    return mapped_value

def main():
    start_time = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    params = BrainFlowInputParams()
    params.serial_port = "COM10"
    board = BoardShim(BoardIds.CYTON_BOARD, params)
    
    board.prepare_session()
    board.start_stream()
    
    # Initialize serial communication with Arduino Nano
    arduino = serial.Serial('COM5', 9600)  # Replace 'COM5' with your Arduino's port

    while True:
        time.sleep(5)  # Collect 5 seconds of data (1250 samples at 250 Hz)
        data = board.get_board_data()
        
        transposed_data = data.T
        selected_data = transposed_data[:, 1:7]  # Assuming columns 1-6 correspond to EEG channels
        
        filtered_data = high_pass_filter(selected_data.T).T
        
        agr_values, avg_agr = compute_agr(filtered_data.T)
        
        mapped_value = map_agr_to_range(avg_agr)
        
        # Send mapped value to Arduino via serial communication
        arduino.write(f"{mapped_value}\n".encode()) 
        
        print(f"AGR values per channel: {agr_values}")
        print(f"Average AGR across channels: {avg_agr} (Mapped: {mapped_value})")
        
        filename = f'brainflow_data_{start_time}.csv'
        with open(filename, 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(filtered_data.T)
            writer.writerow(['AGR'] + agr_values + [avg_agr])
    
    board.stop_stream()
    board.release_session()
    arduino.close()

if __name__ == "__main__":
    main()