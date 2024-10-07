from flask import Flask, request, jsonify
import pandas as pd

app = Flask(__name__)

# Endpoint to receive the data from ESP32
@app.route('/data', methods=['POST'])
def receive_data():
    data = request.json
    print(data)
    
    # Save the received data into a CSV file
    df = pd.DataFrame([data])
    df.to_csv('energy_data.csv', mode='a', header=False, index=False)
    
    return jsonify({"status": "success", "data_received": data})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
