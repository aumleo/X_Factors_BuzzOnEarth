import pandas as pd
import json
import requests 

# Load pin values from the saved JSON file
with open('pin_values.json', 'r') as f:
    pin_values_dict = json.load(f)

# Convert dictionary to DataFrame
pin_values_df = pd.DataFrame(pin_values_dict)

# Perform calculations
total_energy = pin_values_df['V3'].sum()
total_power = pin_values_df['V2'].sum()

# Set baseline values
baseline_energy = 26  # kWh #OR WE CAN SET VALUES OURSELVES / ACCORDING TO THE USER AS TO WHAT THE DESIRED CUTOFF SHOULD BE
baseline_power = 10 # kW

# Generate recommendations
recommendations = []

if total_energy > baseline_energy:
    recommendations.append("Your energy consumption is higher than the average. Consider implementing energy-saving measures.")
    recommendations.append("Remember to turn off appliances when not in use to save energy.")

if total_power > baseline_power:
    recommendations.append("Your power consumption is higher than the average. Monitor high-power appliances and consider energy-efficient alternatives.")


# Send recommendations to Blynk

# Function to send recommendations to Blynk
def send_recommendations_to_blynk(recommendations, auth_token):
    # Base URL for Blynk's notification API
    blynk_url = f"https://blynk.cloud/external/api/sendNotification?token={auth_token}"

    # Loop through recommendations and send as notifications
    for idx, recommendation in enumerate(recommendations, start=1):
        # Customize the notification message with the recommendation index and text
        notification_message = f"Recommendation {idx}: {recommendation}"

        # Send the notification to Blynk
        response = requests.get(blynk_url, params={"message": notification_message})

        # Check if the notification was sent successfully
        if response.status_code == 200:
            print(f"Notification {idx} sent successfully to Blynk.")
        # else:
        #     print(f"Failed to send notification {idx} to Blynk. Status code: {response.status_code}")

# TOKEN HIDDEN AGAIN 
auth_token = "3P3MOCTij9_YMXJC8UvHsjSM3V4S####"

send_recommendations_to_blynk(recommendations, auth_token)


# Print recommendations
if recommendations:
    print("Recommendations:")
    for idx, recommendation in enumerate(recommendations, start=1):
        print(f"{idx}. {recommendation}")
else:
    print("No recommendations at this time. Keep up the good work!")
