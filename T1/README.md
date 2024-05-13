## Plantilla T1

### Integrantes

- Francisco Galdames
- Valentina Vásquez
- Vicente Soto

---



_Aqui deben de hacer un readme con la estrucutra y flujo basico de su arquitectura_


## Comandos de docker


### Iniciar la base de datos

```bash
docker compose up -d
```

### Detener la base de datos

```bash
docker compose down
```

### Borrar la base de datos

```bash
docker compose down 
docker volume rm t1_postgres_data_iot
```