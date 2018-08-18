/***********************************
 * Ant Tx Counter Example
 *
 * Opens a channel with the public
 * network key and transmits data
 * increments in value every second
 *
 * Author Curtis Malainey
 ************************************/

#include "ANT.h"
#define BAUD_RATE 9600
Ant ant = Ant();

uint8_t buffer[] = {0, 0, 0, 0, 0, 0, 0, 0};

void parseMessage();
void parseEventMessage(uint8_t code);

void setup()
{
	AssignChannel ac;
	ResetSystem rs;
	SetNetworkKey snk;
	ChannelId ci;
	ChannelPeriod cp;
	ChannelRfFrequency crf;
	BroadcastMsg bm;
	OpenChannel oc;

	Serial1.begin(BAUD_RATE);
	// this will be moved into the driver eventually
	#if defined(CORE_TEENSY)
		Serial1.attachCts(20);
	#else
		// ant.attachCts()
	#endif
	ant.setSerial(Serial1);
	ant.send(rs);
	// Delay after resetting the radio to give the user time to connect on serial
	delay(10000);
	Serial.println("Running");
	parseMessage();

	ac = AssignChannel();
	ac.setChannel(0);
	ac.setChannelType(CHANNEL_TYPE_BIDIRECTIONAL_TRANSMIT); //can't wildcard this
	ac.setChannelNetwork(0);
	ant.send(ac);
	parseMessage();

	ci = ChannelId();
	ci.setChannel(0);
	ci.setDeviceNumber(0);
	ci.setDeviceType(0);
	ci.setTransmissionType(0);
	ant.send(ci);
	parseMessage();

	cp = ChannelPeriod();
	cp.setChannel(0);
	cp.setPeriod(1234); //can't wildcard this
	ant.send(cp);
	parseMessage();

	crf = ChannelRfFrequency();
	crf.setChannel(0);
	crf.setRfFrequency(0); //can't wildcard this
	ant.send(crf);
	parseMessage();

	bm = BroadcastMsg();
	bm.setData(buffer);
	bm.setChannel(0);
	ant.send(bm);
	parseMessage();

	oc = OpenChannel();
	oc.setChannel(0);
	ant.send(oc);
	parseMessage();
}

void loop()
{
	parseMessage();
}

void parseMessage() {
	ant.readPacket();
	if(ant.getResponse().isAvailable())
	{
		uint8_t msgId = ant.getResponse().getMsgId();
		switch (msgId) {
			case CHANNEL_EVENT:
			{
				ChannelEventResponse cer = ChannelEventResponse();
				ant.getResponse().getChannelEventResponseMsg(cer);
				Serial.println("Received Msg: ChannelEventResponse");
				Serial.print("Channel: ");
				Serial.println(cer.getChannelNumber());
				parseEventMessage(cer.getCode());
				break;
			}

			case START_UP_MESSAGE:
			{
				StartUpMessage sum = StartUpMessage();
				ant.getResponse().getStartUpMsg(sum);
				Serial.println("Received Msg: StartupMessage");
				Serial.print("Message: ");
				Serial.println(sum.getMessage());
				break;
			}

			default:
				Serial.print("Undefined Message: ");
				Serial.println(msgId, HEX);
				break;
		}
	}
	else if (ant.getResponse().isError())
	{
		Serial.print("ANT MSG ERROR: ");
		Serial.println(ant.getResponse().getErrorCode());
	}
}

void parseEventMessage(uint8_t code)
{
	BroadcastMsg bm;
	Serial.print("Code: ");
	switch (code)
	{
		case RESPONSE_NO_ERROR:
			Serial.println("RESPONSE_NO_ERROR");
			break;

		case EVENT_CHANNEL_CLOSED:
			Serial.println("EVENT_CHANNEL_CLOSED");
			break;

		case EVENT_TX:
			Serial.println("EVENT_TX");
			buffer[0]++;
			bm = BroadcastMsg();
			bm.setData(buffer);
			bm.setChannel(0);
			ant.send(bm);
			break;

		default:
			Serial.println(code);
			break;
	}
}