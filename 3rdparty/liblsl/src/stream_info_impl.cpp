#include <boost/thread/lock_guard.hpp>
#include "cast.h"
#include "stream_info_impl.h"
#include "api_config.h"


// === implementation of the stream_info_impl class ===

using namespace lsl;
using namespace pugi;
using std::string;

/// Default Constructor.
stream_info_impl::stream_info_impl(): channel_count_(0), nominal_srate_(0), channel_format_(cft_undefined), version_(0), v4data_port_(0), v4service_port_(0), v6data_port_(0), v6service_port_(0), created_at_(0) {
	// initialize XML document
	write_xml(doc_);
}

/// Constructor.
stream_info_impl::stream_info_impl(const string &name, const string &type, int channel_count, double nominal_srate, lsl_channel_format_t channel_format, const string &source_id):
	name_(name), type_(type), channel_count_(channel_count), nominal_srate_(nominal_srate), channel_format_(channel_format), source_id_(source_id), version_(api_config::get_instance()->use_protocol_version()),
	v4data_port_(0), v4service_port_(0), v6data_port_(0), v6service_port_(0), created_at_(0) {
	if (name.empty())
		throw std::invalid_argument("The name of a stream must be non-empty.");
	if (channel_count < 0)
		throw std::invalid_argument("The channel_count of a stream must be nonnegative.");
	if (nominal_srate < 0)
		throw std::invalid_argument("The nominal sampling rate of a stream must be nonnegative.");
	if (channel_format < 0 || channel_format > 7)
		throw std::invalid_argument("The stream info was created with an unknown channel format.");
	// initialize XML document
	write_xml(doc_);
}

/// Helper function to add a child node with only a single text node
template<typename T> void append_text_node(xml_node& node, const char* name, const T& value) {
	node.append_child(name).append_child(node_pcdata).text().set(value);
}

template<> void append_text_node(xml_node& node, const char* name, const std::string& value) {
	node.append_child(name).append_child(node_pcdata).set_value(value.c_str());
}


/// Initialize the XML DOM structure (leaving .desc unchanged) from the data.
void stream_info_impl::write_xml(xml_document &doc) {
	const char *channel_format_strings[] = {"undefined","float32","double64","string","int32","int16","int8","int64"};
	xml_node info = doc.append_child("info");
	append_text_node(info, "name", name_);
	append_text_node(info, "type", type_);
	append_text_node(info, "channel_count", channel_count_);
	append_text_node(info, "channel_format", channel_format_strings[channel_format_]);
	append_text_node(info, "source_id", source_id_);
	// floating point fields: use locale independent to_string function
	append_text_node(info, "nominal_srate", to_string(nominal_srate_));
	append_text_node(info, "version", to_string(version_ / 100.));
	append_text_node(info, "created_at", to_string(created_at_));
	append_text_node(info, "uid", uid_);
	append_text_node(info, "session_id", session_id_);
	append_text_node(info, "hostname", hostname_);
	append_text_node(info, "v4address", v4address_);
	append_text_node(info, "v4data_port", v4data_port_);
	append_text_node(info, "v4service_port", v4service_port_);
	append_text_node(info, "v6address", v6address_);
	append_text_node(info, "v6data_port", v6data_port_);
	append_text_node(info, "v6service_port", v6service_port_);
	info.append_child("desc");
}

/// Read & assign the object's fields from an XML DOM structure.
void stream_info_impl::read_xml(xml_document &doc) {
	try {
		xml_node info = doc.child("info");
		// name
		name_ = info.child_value("name");
		if (name_.empty())
			throw std::runtime_error("Received a stream info with empty <name> field.");
		// type
		type_ = info.child_value("type");
		// channel_count
		channel_count_ = std::stoi(info.child_value("channel_count"));
		if (channel_count_ < 0)
			throw std::runtime_error("The channel count of the given stream info is smaller than 0.");
		// nominal_srate
		nominal_srate_ = from_string<double>(info.child_value("nominal_srate"));
		if (nominal_srate_ < 0.0)
			throw std::runtime_error("The sampling rate of the given stream info is negative.");
		// channel_format
		channel_format_ = cft_undefined;
		string fmt(info.child_value("channel_format"));
		if (fmt == "float32")
			channel_format_ = cft_float32;
		if (fmt == "double64")
			channel_format_ = cft_double64;
		if (fmt == "string")
			channel_format_ = cft_string;
		if (fmt == "int32")
			channel_format_ = cft_int32;
		if (fmt == "int16")
			channel_format_ = cft_int16;
		if (fmt == "int8")
			channel_format_ = cft_int8;
		if (fmt == "int64")
			channel_format_ = cft_int64;
		// source_id
		source_id_ = info.child_value("source_id");
		// version
		version_ = (int)(from_string<double>(info.child_value("version")) * 100.0);
		if (version_ <= 0)
			throw std::runtime_error("The version of the given stream info is invalid.");
		// created_at
		created_at_ = from_string<double>(info.child_value("created_at"));
		// uid
		uid_ = info.child_value("uid");
		if (uid_.empty())
			throw std::runtime_error("The UID of the given stream info is empty.");
		// session_id
		session_id_ = info.child_value("session_id");
		// hostname
		hostname_ = info.child_value("hostname");
		// address
		v4address_ = info.child_value("v4address");
		// data_port
		v4data_port_ = std::stoi(info.child_value("v4data_port"));
		// service_port
		v4service_port_ = std::stoi(info.child_value("v4service_port"));
		// address
		v6address_ = info.child_value("v6address");
		// data_port
		v6data_port_ = std::stoi(info.child_value("v6data_port"));
		// service_port
		v6service_port_ = std::stoi(info.child_value("v6service_port"));
	} catch(std::exception &e) {
		// reset the stream info to blank state
		*this = stream_info_impl();
		name_ = (string("(invalid: ") += e.what()) += ")";
	}
}

//
// === Protocol Support Operations Implementation ===
//

/**
* Get the short-info message according to this stream_info.
* The short-info message is a shortened xml representation of the stream_info, excluding the .desc field (which can be megabytes in size).
* This message is sent by a stream outlet in response to a variety of queries.
*/
string stream_info_impl::to_shortinfo_message() {
	// make a new document (with an empty <desc> field)
	xml_document tmp; write_xml(tmp);
	// write it to a stream
	std::ostringstream os; tmp.save(os);
	// and get the string
	return os.str();
}

/**
* Initialize a stream_info from a short-info message .
* This functions resets all fields of the stream_info accoridng to the message. The .desc field will be empty.
*/
void stream_info_impl::from_shortinfo_message(const std::string &m) {
	// load the doc from the message string
	doc_.load_buffer(m.c_str(),m.size());
	// and assign all the struct fields, too...
	read_xml(doc_);
}

/**
* Get the full-info message for this stream_info.
* This is a complete XML representation of the stream_info.
*/
std::string stream_info_impl::to_fullinfo_message() {
	// write the doc to a stream
	std::ostringstream os; doc_.save(os);
	// and get the string
	return os.str();
}

/**
* Initialize a stream_info from a full-info message.
* This functions resets all fields of the stream_info accoridng to the message.
*/
void stream_info_impl::from_fullinfo_message(const std::string &m) {
	// load the doc from the message string
	doc_.load_buffer(m.c_str(),m.size());
	// and assign all the struct fields, too...
	read_xml(doc_);
}

/// Test whether this stream info matches the given query string.
bool stream_info_impl::matches_query(const string &query, bool nocache) {
	return cached_.matches_query(doc_, query, nocache);
}

bool query_cache::matches_query(const xml_document &doc, const std::string query, bool nocache) {
	lslboost::lock_guard<lslboost::mutex> lock(cache_mut_);

	decltype(cache)::iterator it;
	if (!nocache && (it = cache.find(query)) != cache.end()) {
		// the sign bit encodes if the query matches or not
		bool matches = it->second > 0;
		// update the last-use time stamp
		it->second = ++query_cache_age * (matches ? 1 : -1);
		// return cached match
		return matches;
	}

	// not found in cache
	try {
		// compute whether it matches
		bool matched = pugi::xpath_query(query.c_str()).evaluate_boolean(doc.first_child());

		auto max_cached = (std::size_t)api_config::get_instance()->max_cached_queries();
		if(nocache || max_cached == 0)
			return matched;

		cache.insert(std::make_pair(query, ++query_cache_age * (matched ? 1 : -1)));

		// remove n/2 oldest results to make room for new entries
		if (cache.size() > max_cached) {
			// Find out the median cache entry age
			std::vector<int> agevec;
			agevec.reserve(cache.size());
			for (auto &val : cache) agevec.push_back(std::abs(val.second));
			auto middle = agevec.begin() + max_cached / 2;
			std::nth_element(agevec.begin(), middle, agevec.end());
			auto oldest_to_keep = *middle;

			// Remove all elements older than the median age
			for (auto it = cache.begin(); it != cache.end();)
				if (abs(it->second) <= oldest_to_keep)
					it = cache.erase(it);
				else
					++it;
		}
		return matched;
	} catch (std::exception &e) {
		LOG_F(WARNING, "Query error: %s", e.what());
		return false;
	}
}

int stream_info_impl::channel_bytes() const
{
	const int channel_format_sizes[] = {0,sizeof(float),sizeof(double),sizeof(std::string),sizeof(int32_t),sizeof(int16_t),sizeof(int8_t),8};
	return channel_format_sizes[channel_format_];
}

/**
* Return a handle to the info/desc element.
*/
xml_node stream_info_impl::desc() {return doc_.child("info").child("desc"); }
xml_node stream_info_impl::desc() const { return doc_.child("info").child("desc"); }

/**
* Set the info / protocol version used by the stream.
*/
void stream_info_impl::version(int v) { 
	version_ = v;
	doc_.child("info").child("version").first_child().set_value(to_string(version_ / 100.).c_str());
}

/**
* Set the creation time stamp of a stream.
* This is the time stamp (via now()) of when the stream was first created
* (in the time domain of the providing machine).
*/
void stream_info_impl::created_at(double v) { 
	created_at_ = v; 
	doc_.child("info").child("created_at").first_child().set_value(to_string(created_at_).c_str());
}

/**
* Set the UID of a stream instance (once assigned).
* This is a unique identifier of the stream instance, and is guaranteed to be different
* across multiple instantiations of the same stream (e.g., after a re-start).
*/
void stream_info_impl::uid(const std::string &v) { 
	uid_ = v; 
	doc_.child("info").child("uid").first_child().set_value(uid_.c_str()); 
}

/**
* Set the session id for the given stream.
* The session ID is an optional human-assigned identifier of the recording session; only
* inlets and outlets that have the same session id can be paired with each other to avoid
* accidentally recording from an unrelated concurrent session on the same network.
* The session id can be set via the configuration file (see Network Connectivity in the LSL wiki).
*/
void stream_info_impl::session_id(const std::string &v) { 
	session_id_ = v; 
	doc_.child("info").child("session_id").first_child().set_value(session_id_.c_str()); 
}

/**
* Set the provider hostname for the given stream.
*/
void stream_info_impl::hostname(const std::string &v) { 
	hostname_ = v; 
	doc_.child("info").child("hostname").first_child().set_value(hostname_.c_str()); 
}

/**
* Set the host name or IP address where the stream is hosted.
*/
void stream_info_impl::v4address(const std::string &v) { 
	v4address_ = v; 
	doc_.child("info").child("v4address").first_child().set_value(v4address_.c_str()); 
}

/**
* Set the TCP data port where the stream is hosted (once assigned).
* This port is internally used to obtain data and meta-data from a stream.
*/
void stream_info_impl::v4data_port(uint16_t v) {
	v4data_port_ = v; 
	doc_.child("info").child("v4data_port").first_child().text().set(v4data_port_);
}

/**
* Set the UDP service port where the stream is hosted (once assigned).
* This port is internally used to obtain time correction information for a stream.
*/
void stream_info_impl::v4service_port(uint16_t v) {
	v4service_port_ = v; 
	doc_.child("info").child("v4service_port").first_child().text().set(v4service_port_);
}

/**
* Set the host name or IP address where the stream is hosted.
*/
void stream_info_impl::v6address(const std::string &v) { 
	v6address_ = v; 
	doc_.child("info").child("v6address").first_child().set_value(v6address_.c_str()); 
}

/**
* Set the TCP data port where the stream is hosted (once assigned).
* This port is internally used to obtain data and meta-data from a stream.
*/
void stream_info_impl::v6data_port(uint16_t v) {
	v6data_port_ = v; 
	doc_.child("info").child("v6data_port").first_child().text().set(v6data_port_);
}

/**
* Set the UDP service port where the stream is hosted (once assigned).
* This port is internally used to obtain time correction information for a stream.
*/
void stream_info_impl::v6service_port(uint16_t v) {
	v6service_port_ = v; 
	doc_.child("info").child("v6service_port").first_child().text().set(v6service_port_);
}

/**
* Assignment operator.
* Needs special handling because xml_document is non-copyable.
*/
stream_info_impl& stream_info_impl::operator=(stream_info_impl const &rhs) {
	if (this == &rhs)
		return *this;
	name_ = rhs.name_;
	type_ = rhs.type_;
	channel_count_ = rhs.channel_count_;
	nominal_srate_ = rhs.nominal_srate_;
	channel_format_ = rhs.channel_format_;
	source_id_ = rhs.source_id_;
	version_ = rhs.version_;
	v4address_ = rhs.v4address_;
	v4data_port_ = rhs.v4data_port_;
	v4service_port_ = rhs.v4service_port_;
	v6address_ = rhs.v6address_;
	v6data_port_ = rhs.v6data_port_;
	v6service_port_ = rhs.v6service_port_;
	uid_ = rhs.uid_;
	created_at_ = rhs.created_at_;
	session_id_ = rhs.session_id_;
	hostname_ = rhs.hostname_;
	doc_.reset(rhs.doc_);
	return *this;
}

/**
* Copy constructor.
* Needs special handling because xml_document is non-copyable.
*/
stream_info_impl::stream_info_impl(const stream_info_impl &rhs): name_(rhs.name_), type_(rhs.type_), channel_count_(rhs.channel_count_),
nominal_srate_(rhs.nominal_srate_), channel_format_(rhs.channel_format_), source_id_(rhs.source_id_), version_(rhs.version_), v4address_(rhs.v4address_),
v4data_port_(rhs.v4data_port_), v4service_port_(rhs.v4service_port_), v6address_(rhs.v6address_), v6data_port_(rhs.v6data_port_), v6service_port_(rhs.v6service_port_),
uid_(rhs.uid_), created_at_(rhs.created_at_), session_id_(rhs.session_id_), hostname_(rhs.hostname_) {
	doc_.reset(rhs.doc_);
}

