import json
import pandas as pd
import requests

# URL de la API REST de TTN
url = f"https://eu1.cloud.thethings.network/api/v3/as/applications/gps-unex-tfm/packages/storage/uplink_message"

# API Key de la API
api_key = "NNSXS.EBHL343LMRI4IPKK36LMGHFR473NJJDV6J6EFHA.X4G6XCVW4IIHBQ6P6LARA37BT3JQ523CKEEBN3WU6EVMHFYHFX5Q"

# Encabezado de acceso a la API
headers = {
    "Authorization": "Bearer " + api_key
}

# Solicitud GET a la API
response = requests.get(url, headers=headers)

def Extraction():

    # Dataframe vacío con el nombre de la información que queremos extraer
    dataframe = pd.DataFrame(columns=['time', 'altitude', 'latitud', 'longitud', 'satellites', 'speed'])

    # Comprueba si se la solicitud ha sido correcta
    if response.status_code == 200:
        try:
            # Extraemos toda la información y dividimos los mensajes según los saltos de linea
            response_lines = response.text.split("\n")

            # Contador para recorrer el dataframe vacío response_lines
            i = 0

            # Recorremos cada linea línea y se analizan sabiendo que tiene formato JSON
            for line in response_lines:

                # Ignorar las líneas vacías (última)
                if line.strip() == "":
                    continue

                # Se analiza la línea como JSON
                data = json.loads(line)

                # Se guarda la altitud, latitud, longitud, velocidad y nº de satélites
                dataframe.loc[i] = data["result"]["uplink_message"]["decoded_payload"]

                # Fecha y tiempo de recepción del mensaje
                dataframe.loc[i, "time"] = data["result"]["received_at"]

                # Incremento de contador
                i += 1
            print("Todos los mensajes procesados correctamente")

        except json.JSONDecodeError as e:
            print("Error de decodificación JSON:", e)

    else:
        print("Error al obtener los datos de la API:", response.status_code)

    return dataframe
