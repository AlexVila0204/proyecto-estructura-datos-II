# Proyecto de Estructura de Datos II - Sistema de Archivos Binarios

Este proyecto implementa un sistema de gestión de archivos binarios que permite realizar operaciones CRUD (Crear, Leer, Actualizar, Eliminar) sobre registros almacenados en archivos binarios. El sistema soporta búsquedas por llave primaria y secundaria, así como operaciones de compactación y descripción de la estructura de los archivos.

## Características

- Creación y gestión de archivos binarios
- Carga de datos desde archivos CSV
- Búsqueda por llave primaria (PK) y secundaria (SK)
- Operaciones CRUD completas
- Compactación de archivos
- Descripción de la estructura de archivos
- Soporte para diferentes tipos de datos

## Requisitos

- Compilador C++ (g++ o compatible)
- CMake (opcional)
- Biblioteca nlohmann/json para manejo de JSON

## Estructura del Proyecto

```
.
├── data/               # Archivos de datos de prueba
│   ├── customers.csv
│   ├── employees.csv
│   ├── friends.csv
│   ├── products.csv
│   └── ...
├── nlohmann/          # Biblioteca JSON
├── main.cpp           # Programa principal
├── ReadFunction.h     # Encabezado para funciones de lectura
├── ReadFunction.cpp   # Implementación de funciones de lectura
├── WriteFunction.h    # Encabezado para funciones de escritura
└── WriteFunction.cpp  # Implementación de funciones de escritura
```

## Compilación

### Usando CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Compilación directa con g++
```bash
g++ main.cpp ReadFunction.cpp WriteFunction.cpp -o programa
```

## Uso del Programa

El programa se ejecuta desde la línea de comandos con diferentes argumentos según la operación deseada:

### Crear un archivo JSON
```bash
./programa -create archivo.json
```

### Cargar datos desde CSV
```bash
./programa -file archivo.bin -load datos.csv
```

### Ver contenido de archivo binario
```bash
./programa -file archivo.bin -GET
```

### Buscar por llave primaria
```bash
./programa -file archivo.bin -GET -pk -value=<valor>
```

### Buscar por llave secundaria
```bash
./programa -file archivo.bin -GET -sk=<campo> -value=<valor>
```

### Agregar nuevo registro
```bash
./programa -file archivo.bin -POST -data={<datos_en_formato_json>}
```

### Modificar registro existente
```bash
./programa -file archivo.bin -PUT -pk=<valor> -data={<datos_en_formato_json>}
```

### Eliminar registro
```bash
./programa -file archivo.bin -DELETE -pk=<valor>
```

### Compactar archivo
```bash
./programa -file archivo.bin -compact
```

### Describir estructura del archivo
```bash
./programa -file archivo.bin -describe
```

## Ejemplos de Uso

### Cargar datos de prueba
```bash
./programa -file data/friends.bin -load data/friends.csv
```

### Ver estructura del archivo
```bash
./programa -file data/friends.bin -describe
```

### Buscar un amigo por ID
```bash
./programa -file data/friends.bin -GET -pk -value=1
```

## Archivos de Datos de Prueba

El proyecto incluye varios archivos de datos de prueba en el directorio `data/`:

- `friends.csv`: Datos de amigos con campos id, name, age, sex
- `products.csv`: Catálogo de productos con diferentes campos
- `customers.csv`: Información de clientes
- `employees.csv`: Datos de empleados

## Notas

- Los archivos binarios (.bin) contienen los datos estructurados
- Los archivos .idx son índices para búsquedas rápidas
- Los archivos .sdx son índices secundarios
- Los archivos .json contienen la estructura de los datos

## Autor

- Alberth Alexander Godoy Avila
- Carné: 12111345
- Universidad Tecnológica Centroamericana (UNITEC)
