#include "PocoOpenAiClient.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include <iostream>
#include <sstream>

using namespace Poco::Net;
using namespace Poco;
using namespace Poco::JSON;
using namespace std;

std::string getEnv(std::string envVar)
{
	char* buf = nullptr;
	size_t sz = 0;
	std::string ret;
	if (_dupenv_s(&buf, &sz, envVar.c_str()) == 0 && buf != nullptr)
	{
		//printf("EnvVarName = %s\n", buf);
		ret = (buf);
		free(buf);
	}
	return ret;
}

OpenAIClient::OpenAIClient(const std::string& modelName)
	: model(modelName) {
	Poco::Net::initializeSSL();
	apiKey = getEnv("OPENAI_API_KEY");;
	// Create a context that doesn't verify certificates
	Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pCertHandler = new Poco::Net::AcceptCertificateHandler(false);
	Poco::Net::Context::Ptr pContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_NONE);
	Poco::Net::SSLManager::instance().initializeClient(0, pCertHandler, pContext);

}

std::string OpenAIClient::generateCompletion(const std::string& prompt) {
	try {

		// Create URI
		URI uri("https://api.openai.com/v1/chat/completions");

		// Set up SSL context
		Context::Ptr context = new Context(Context::CLIENT_USE, "", "", "", Context::VERIFY_RELAXED);

		// Create HTTPS session
		HTTPSClientSession session(uri.getHost(), uri.getPort(), context);

		// Prepare request
		HTTPRequest request(HTTPRequest::HTTP_POST, uri.getPathAndQuery(), HTTPMessage::HTTP_1_1);
		request.setContentType("application/json");
		request.set("Authorization", "Bearer " + apiKey);

		// Create JSON payload
		Object requestObj;
		requestObj.set("model", model);

		Array messagesArray;
		Object messageObj;
		messageObj.set("role", "user");
		messageObj.set("content", prompt);
		messagesArray.add(messageObj);

		requestObj.set("messages", messagesArray);

		// Convert to string and send
		ostringstream oss;
		requestObj.stringify(oss);
		request.setContentLength(oss.str().length());

		ostream& os = session.sendRequest(request);
		os << oss.str();

		// Get response
		HTTPResponse response;
		istream& rs = session.receiveResponse(response);

		// Parse JSON response
		Parser parser;
		Dynamic::Var result = parser.parse(rs);
		Object::Ptr resultObj = result.extract<Object::Ptr>();

		Array::Ptr choices = resultObj->getArray("choices");
		if (choices->size() > 0) {
			Object::Ptr choice = choices->getObject(0);
			Object::Ptr message = choice->getObject("message");
			return message->getValue<std::string>("content");
		}

		return "No response generated";
	}
	catch (Exception& ex) {
		cerr << "Error: " << ex.displayText() << endl;
		return "Error: " + ex.displayText();
	}
}