# text-search
# Todos:
# Timer und Benchmarks:
Es macht vielleicht Sinn eine Klasse zu implementieren, welche Informationen über die Benchmark Tests zusammenfasst.
Zum Beispiel wird bei der Ausführung der main() function gespeichert welche Suche lief (bspw: Standard lib find()) welcher String gesucht wurde, welche Zeit und so weiter.
Idealerweise kann man später das ganze sogar mit python plotten.

# OpenMP
verschiedene Threads

# OpenMPI
verschiedene Prozess

# OpenCL 
eufteilen auf GPU threads

# Hybrid-Modells (zusatz)
Aufteilen der auf prozesse welche mit mehreren Threads arebeiten

# Weitere text-search ansätze
## Hashing (max)
Idee: Texte oder Textteile werden in Hash-Werte umgewandelt. Suche prüft dann nur die Hashes statt den gesamten Text.
Vorteil: Sehr schnelle Suche bei exakten Übereinstimmungen.
Beispiele: Hash-Tabellen, Rabin-Karp Algorithmus für String Matching.

