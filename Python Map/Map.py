import Extraction
import folium
import webbrowser

# Llamamos al script de extracción de datos de la API
data = Extraction.Extraction()

# Creamos un mapa centrado en Cáceres
mapa = folium.Map(location=[39.47671305350662, -6.370001317987229], zoom_start=50)

# Marcadores para cada ubicación en el DataFrame
for index, row in data.iterrows():
    folium.Marker([row['latitud'], row['longitud']], popup=f"<b>Time:</b> {row['time']}<br><b>Altitud:</b> {row['altitude']}<br><b>Latitud:</b> {row['latitud']}<br><b>Longitud:</b> {row['longitud']}<br><b>Satélites:</b> {row['satellites']}<br><b>Velocidad:</b> {row['speed']}").add_to(mapa)

# Guardamos y abrimos el mapa como un archivo HTML
mapa.save("mapa.html")
webbrowser.open("mapa.html")
