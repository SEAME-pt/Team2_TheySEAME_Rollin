#include "gtest/gtest.h"
#include "Subject.hpp"
#include "Mocks.h"

using namespace testing;

class SubjectTest : public Test {
protected:
	Subject subject;
};

TEST_F(SubjectTest, AddAndNotifyObserver) {
	MockObserver obs;

	subject.attach(&obs);
	EXPECT_CALL(obs, update(Events::CAR_START)).Times(1);
	subject.notify(Events::CAR_START);
}

TEST_F(SubjectTest, RemoveAndNotifyObserver) {
	MockObserver obs;

	subject.attach(&obs);
	subject.detach(&obs);
	EXPECT_CALL(obs, update(Events::CAR_START)).Times(0);
	subject.notify(Events::CAR_START);
}

TEST_F(SubjectTest, AddAndNotifyMoreThanOneObserver) {
	MockObserver obs1;
	MockObserver obs2;

	subject.attach(&obs1);
	subject.attach(&obs2);
	EXPECT_CALL(obs1, update(Events::CAR_START)).Times(1);
	EXPECT_CALL(obs2, update(Events::CAR_START)).Times(1);
	subject.notify(Events::CAR_START);
}

TEST_F(SubjectTest, RemovingMoreThanOneObserver) {
	MockObserver obs1;
	MockObserver obs2;

	subject.attach(&obs1);
	subject.attach(&obs2);
	subject.detach(&obs1);
	subject.detach(&obs2);
	EXPECT_CALL(obs1, update(Events::CAR_START)).Times(0);
	EXPECT_CALL(obs2, update(Events::CAR_START)).Times(0);
	subject.notify(Events::CAR_START);
}

TEST_F(SubjectTest, ObserverArrayLimit) {
	MockObserver obs;
	MockObserver postLimitObs;

	for (int i = 0; i < subject.size; i++) {
		subject.attach(&obs);
	}
	subject.attach(&postLimitObs);
	EXPECT_CALL(obs, update(Events::CAR_START)).Times(subject.size);
	EXPECT_CALL(postLimitObs, update(Events::CAR_START)).Times(0);
	subject.notify(Events::CAR_START);
}
