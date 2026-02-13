FROM eclipse-temurin:21

WORKDIR /app

COPY docs/requirements/*.md requirements/
<<<<<<< HEAD
=======
COPY docs/requirements/modules/*.md requirements/
>>>>>>> 92140775 (Release/1.0.0 (#335))
COPY apps/ src/

RUN apt-get update && apt-get install -y curl \
    && curl -L -o oft.jar https://github.com/itsallcode/openfasttrace/releases/download/4.2.0/openfasttrace-4.2.0.jar \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["java", "-jar", "oft.jar"]
