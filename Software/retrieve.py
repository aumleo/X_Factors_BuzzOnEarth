import requests
import json

def get_blynk_pin_values(auth, pins):
    base_url = f'https://{sadd}/external/api/get?token={auth}&'
    pin_values = {pin: [] for pin in pins}

    for pin in pins:
        url = base_url + pin
        response = requests.get(url, verify=False)
        if response.status_code == 200:
            data = response.json()
            pin_values[pin].append(data)
        else:
            print(f"Failed to get value from Blynk for pin {pin}. Status code: {response.status_code}")

    # Load existing pin values from the JSON file, if it exists
    try:
        with open('pin_values.json', 'r') as f:
            existing_pin_values = json.load(f)
    except FileNotFoundError:
        existing_pin_values = {}

    # Merge the existing pin values with the new pin values
    for pin, values in pin_values.items():
        existing_pin_values.setdefault(pin, []).extend(values)

    # Save the merged pin values back to the JSON file
    with open('pin_values.json', 'w') as f:
        json.dump(existing_pin_values, f)


# KEYS ARE HIDDEN FOR SECURITY SO CODE WONT WORK FOR NOW, NEED ACTUAL ID'S AND TOKENS
sadd = '192.168.186.###'
auth = "TMPL3cvqp####"

pins = ["V0", "V1", "V2", "V3"]  # List of pin numbers you want to retrieve values from

get_blynk_pin_values(auth, pins)

