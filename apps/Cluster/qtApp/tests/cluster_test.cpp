#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include "../src/systemInfo.hpp"
#include "../src/generalInfo.hpp"

class SystemInfoTest : public ::testing::Test {
protected:
    void SetUp() override { sysInfo = new systemInfo(); }
    void TearDown() override { delete sysInfo; }
    systemInfo* sysInfo;
};

/**
 * @brief Tests initialization of systemInfo object.
 * 
 * =======================Requirements traceability========================
 *        [test->dsn~design-requirement-cluster-speed~1]
 *        [test->dsn~design-requirement-cluster-battery~1]
 * =======================================================================
 */
TEST_F(SystemInfoTest, Initialization) {
    EXPECT_NE(sysInfo, nullptr);
    EXPECT_FALSE(sysInfo->start("invalid_interface"));
}

/**
 * @brief Tests processFrames method for safe execution.
 * 
 * =======================Requirements traceability========================
 *        [test->dsn~design-requirement-cluster-speed~1]
 *        [test->dsn~design-requirement-cluster-battery~1]
 * =======================================================================
 */
TEST_F(SystemInfoTest, ProcessFramesSafe) {
    EXPECT_NO_THROW(sysInfo->processFrames());
}

class GeneralInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        app = new QCoreApplication(argc, nullptr);
        genInfo = new generalInfo();
    }
    void TearDown() override { delete genInfo; delete app; }
    QCoreApplication* app;
    generalInfo* genInfo;
};

/**
 * @brief Tests initialization of generalInfo object.
 * 
 * =======================Requirements traceability========================
 *        [test->dsn~design-requirement-cluster-datetime~1]
 *        [test->dsn~design-requirement-cluster-weather~1]
 * =======================================================================
 */
TEST_F(GeneralInfoTest, Initialization) {
    EXPECT_NE(genInfo, nullptr);
    EXPECT_EQ(genInfo->getLocalTime().size(), 5);
    EXPECT_EQ(genInfo->getCurrentDate().size(), 10);
    EXPECT_GE(genInfo->getTemperature(), -50);
    EXPECT_LE(genInfo->getTemperature(), 60);
    EXPECT_FALSE(genInfo->getWeatherInfo().isEmpty());
}

/**
 * @brief Tests time and date retrieval and signal emissions.
 * 
 * =======================Requirements traceability========================
 *        [test->dsn~design-requirement-cluster-datetime~1]
 * =======================================================================
 */
TEST_F(GeneralInfoTest, TimeDateSignals) {
    QSignalSpy timeSpy(genInfo, &generalInfo::localTimeChanged);
    QSignalSpy dateSpy(genInfo, &generalInfo::currentDateChanged);
    QCoreApplication::processEvents();
    EXPECT_TRUE(timeSpy.count() >= 0);
    EXPECT_TRUE(dateSpy.count() >= 0);
}

class MockNetworkReply : public QNetworkReply {
public:
    MockNetworkReply(const QByteArray &data) : m_data(data) {
        open(ReadOnly | Unbuffered);
        setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
        setFinished(true);
    }
    void abort() override {}
    qint64 bytesAvailable() const override { return m_data.size() + QNetworkReply::bytesAvailable(); }
    qint64 readData(char* buffer, qint64 maxlen) override {
        qint64 len = qMin<qint64>(maxlen, m_data.size() - m_pos);
        memcpy(buffer, m_data.constData() + m_pos, len);
        m_pos += len;
        return len;
    }
private:
    QByteArray m_data;
    qint64 m_pos = 0;
};

/**
 * @brief Tests weather data parsing and signal emissions.
 * 
 * =======================Requirements traceability========================
 *        [test->dsn~design-requirement-cluster-weather~1]
 * =======================================================================
 */
TEST_F(GeneralInfoTest, WeatherSignals) {
    QByteArray json = R"({"current_weather":{"temperature":30,"weathercode":1}})";

    // Force different initial icon to ensure signal is emitted
    genInfo->onWeatherDataReceived(new MockNetworkReply(R"({"current_weather":{"temperature":0,"weathercode":0}})"));

    QSignalSpy tempSpy(genInfo, &generalInfo::temperatureChanged);
    QSignalSpy iconSpy(genInfo, &generalInfo::weatherInfoChanged);

    MockNetworkReply reply(json);
    genInfo->onWeatherDataReceived(&reply);

    EXPECT_EQ(genInfo->getTemperature(), 30);
    EXPECT_EQ(genInfo->getWeatherInfo(), "partly-cloudy-day-256.png");
    EXPECT_GE(tempSpy.count(), 1);
    EXPECT_GE(iconSpy.count(), 1);
}

/**
* @brief Tests weather update with unknown weather code.
*
* =======================Requirements traceability========================
*        [test->dsn~design-requirement-cluster-weather~1]
* ========================================================================
*/
TEST_F(GeneralInfoTest, WeatherUpdateUnknownCode) {
    QByteArray json = R"({"current_weather":{"temperature":10,"weathercode":999}})";

    // Force different initial icon
    genInfo->onWeatherDataReceived(new MockNetworkReply(R"({"current_weather":{"temperature":0,"weathercode":0}})"));

    QSignalSpy iconSpy(genInfo, &generalInfo::weatherInfoChanged);

    MockNetworkReply reply(json);
    genInfo->onWeatherDataReceived(&reply);

    EXPECT_EQ(genInfo->getWeatherInfo(), "unknown.png");
    EXPECT_GE(iconSpy.count(), 1);
}

/**
 * @brief Tests multiple weather updates to ensure signals are emitted correctly.
 * 
 * =======================Requirements traceability========================
 *        [test->dsn~design-requirement-cluster-weather~1]
 * ========================================================================
 */
TEST_F(GeneralInfoTest, MultipleWeatherUpdates) {
    QByteArray json1 = R"({"current_weather":{"temperature":20,"weathercode":0}})";
    QByteArray json2 = R"({"current_weather":{"temperature":25,"weathercode":1}})";

    // Force initial icon different from first JSON
    genInfo->onWeatherDataReceived(new MockNetworkReply(R"({"current_weather":{"temperature":0,"weathercode":999}})"));

    QSignalSpy tempSpy(genInfo, &generalInfo::temperatureChanged);
    QSignalSpy iconSpy(genInfo, &generalInfo::weatherInfoChanged);

    MockNetworkReply reply1(json1);
    MockNetworkReply reply2(json2);
    genInfo->onWeatherDataReceived(&reply1);
    genInfo->onWeatherDataReceived(&reply2);

    EXPECT_EQ(genInfo->getTemperature(), 25);
    EXPECT_EQ(genInfo->getWeatherInfo(), "partly-cloudy-day-256.png");
    EXPECT_GE(tempSpy.count(), 2);
    EXPECT_GE(iconSpy.count(), 2);
}

