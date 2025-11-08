#include <cpp11.hpp>
#include <vector>
#include "samples.h"
#include "pt2-clone.h"
using namespace cpp11;

#define PLAYBACK_FREQ 16574 /* C-3, period 214 */

[[cpp11::register]]
raws open_samp_(raws data) {
  uint8_t * buffer = (uint8_t *)RAW(as_sexp(data));
  uint32_t dat_size = data.size();
  
  moduleSample_t *s = (moduleSample_t *)malloc(sizeof (moduleSample_t));
  if (s == NULL) stop("Out of memory");
  int8_t *smpDat = (int8_t *)malloc(config.maxSampleLength);
  if (smpDat == NULL) {
    free(s);
    stop("Out of memory");
  }
  
  if (!loadSample2(buffer, dat_size, s, smpDat) || s->length == 0) {
    free(s);
    free(smpDat);
    stop("Failed to read sample");
  }
  writable::raws output((R_xlen_t)(s->length));
  
  sexp result = as_sexp(output);
  uint8_t * sampdest = (uint8_t *)RAW(result);
  memcpy(sampdest, smpDat, s->length);
  
  result.attr("class") = "pt2samp";
  result.attr("sample_info") = mod_sample_info_internal2(s);
  
  free(s);
  free(smpDat);
  return output;
}

[[cpp11::register]]
raws sample_file_format_(SEXP input, std::string file_type) {
  raws input_r(input);
  int8_t * ibuffer      = (int8_t *)RAW(input_r);
  list sample_info      = list(input_r.attr("sample_info"));
  uint32_t sampleLength = integers(sample_info["length"]).at(0);
  uint32_t loopStart    = integers(sample_info["loopStart"]).at(0) & ~1;
  uint32_t loopLength   = integers(sample_info["loopLength"]).at(0) & ~1;
  uint32_t volume       = integers(sample_info["volume"]).at(0);
  std::string sampname  = (std::string)strings(sample_info["text"]).at(0);
  uint32_t namelen      = sampname.length();
  
  writable::raws output((R_xlen_t)0);
  
  if (file_type == "IFF") {
    // IFF file size
    // FORM + 8SVX + VHDR + 20 + (NAME + namelen) + ANNO + ProTrackR2 + BODY + samplelen
    // 6 chunk headers of which 5 have also length 6 * 4 + 5 * 4
    // VHDR always has 20 bytes
    // ANNO always has 10 bytes
    // NAME and BODY have variable length
    uint32_t file_size = 6 * 4 + 5 * 4 + 20 + 10 + sampleLength + (sampleLength & 1);
    if (namelen > 0) {
      file_size += (namelen + (namelen&1));
    }
    
    int32_t align = ((uint32_t)(file_size/2))*2;
    
    writable::raws output((R_xlen_t)align);
    uint8_t * buffer = (uint8_t *)RAW(as_sexp(output));
    memcpy(buffer, "FORM", 4);
    buffer += 4;
    ((uint32_t *)buffer)[0] = SWAP32(file_size - 8);
    buffer += 4;
    memcpy(buffer, "8SVXVHDR", 8);
    buffer += 8;
    ((uint32_t *)buffer)[0] = SWAP32(20);
    buffer += 4;
    
    if (loopStart + loopLength > 2) { // loop enabled?
      ((uint32_t *)buffer)[0] = SWAP32(loopStart);
      buffer += 4;
      ((uint32_t *)buffer)[0] = SWAP32(loopLength);
      buffer += 4;
    } else {
      ((uint32_t *)buffer)[0] = SWAP32(sampleLength);
      buffer += 4;
      ((uint32_t *)buffer)[0] = 0;
      buffer += 4;
    }
    
    ((uint32_t *)buffer)[0] = 0;
    buffer += 4;
    ((uint16_t *)buffer)[0] = SWAP16(PLAYBACK_FREQ);
    buffer += 2;
    ((uint8_t *)buffer)[0] = 1;
    buffer += 1;
    ((uint8_t *)buffer)[0] = 0;
    buffer += 1;
    ((uint32_t *)buffer)[0] = SWAP32(volume * 1024);
    buffer += 4;
    
    memcpy(buffer, "NAME", 4);
    buffer += 4;
    ((uint32_t *)buffer)[0] = SWAP32(namelen);
    buffer += 4;
    memcpy(buffer, sampname.c_str(), namelen);
    buffer += namelen;
    if (namelen & 1) {
      buffer[0] = 0;
      buffer++;
    }
    
    memcpy(buffer, "ANNO", 4);
    buffer += 4;
    ((uint32_t *)buffer)[0] = SWAP32(10);
    buffer += 4;
    memcpy(buffer, "ProTrackR2", 10);
    buffer += 10;
    memcpy(buffer, "BODY", 4);
    buffer += 4;
    ((uint32_t *)buffer)[0] = SWAP32(sampleLength);
    buffer += 4;
    memcpy(buffer, ibuffer, sampleLength);
    buffer += sampleLength;
    
    if (sampleLength & 1) buffer[0] = 0;
    output.resize(file_size);
    return output;
  } else if (file_type == "WAV") {
    wavHeader_t wavHeader;
    samplerChunk_t samplerChunk;
    mptExtraChunk_t mptExtraChunk;
    memset(&wavHeader, 0, sizeof(wavHeader_t));
    memset(&samplerChunk, 0, sizeof(samplerChunk_t));
    memset(&mptExtraChunk, 0, sizeof(mptExtraChunk_t));
    
    wavHeader.format = 0x45564157; // "WAVE"
    wavHeader.chunkID = 0x46464952; // "RIFF"
    wavHeader.subchunk1ID = 0x20746D66; // "fmt "
    wavHeader.subchunk2ID = 0x61746164; // "data"
    wavHeader.subchunk1Size = 16;
    wavHeader.subchunk2Size = sampleLength;
    wavHeader.chunkSize = 36 + wavHeader.subchunk2Size;
    wavHeader.audioFormat = 1;
    wavHeader.numChannels = 1;
    wavHeader.bitsPerSample = 8;
    wavHeader.sampleRate = PLAYBACK_FREQ;
    wavHeader.byteRate = wavHeader.sampleRate * wavHeader.numChannels * wavHeader.bitsPerSample / 8;
    wavHeader.blockAlign = wavHeader.numChannels * wavHeader.bitsPerSample / 8;
    
    if (loopStart + loopLength > 2) // loop enabled?
    {
      wavHeader.chunkSize += sizeof (samplerChunk_t);
      samplerChunk.chunkID = 0x6C706D73; // "smpl"
      samplerChunk.chunkSize = 60;
      samplerChunk.dwSamplePeriod = 1000000000 / PLAYBACK_FREQ;
      samplerChunk.dwMIDIUnityNote = 60; // 60 = MIDI middle-C
      samplerChunk.cSampleLoops = 1;
      samplerChunk.loop.dwStart = loopStart;
      samplerChunk.loop.dwEnd = (loopStart + loopLength) - 1;
    }
    
    // set ModPlug Tracker chunk (used for sample volume only in this case)
    wavHeader.chunkSize += sizeof (mptExtraChunk);
    mptExtraChunk.chunkID = 0x61727478; // "xtra"
    mptExtraChunk.chunkSize = sizeof (mptExtraChunk) - 4 - 4;
    mptExtraChunk.defaultPan = 128; // 0..255
    mptExtraChunk.defaultVolume = volume * 4; // 0..256
    mptExtraChunk.globalVolume = 64; // 0..64
    
    uint32_t tot_size =
      sizeof(wavHeader) + sampleLength + (sampleLength & 1) +
      sizeof (mptExtraChunk);
    if (loopStart + loopLength > 2) tot_size += sizeof(samplerChunk);
    
    writable::raws output((R_xlen_t)tot_size);
    uint8_t * buffer = (uint8_t *)RAW(as_sexp(output));
    
    memcpy(buffer, &wavHeader, sizeof(wavHeader));
    buffer += sizeof(wavHeader_t);
    
    for (uint32_t i = 0; i < sampleLength; i++) {
      buffer[0] = (uint8_t)(ibuffer[i] + 128);
      buffer++;
    }
    
    if (sampleLength & 1) {
      buffer[0] = 0;
      buffer++;
    }
    
    if (loopStart+loopLength > 2) {  // loop enabled?
      memcpy(buffer, &samplerChunk, sizeof (samplerChunk));
      buffer += sizeof (samplerChunk);
    }
    
    memcpy(buffer, &mptExtraChunk, sizeof (mptExtraChunk));
    return output;
  } else {
    stop("Writing file type '%s' is not supported.",
         file_type.c_str());
  }
  return R_NilValue;
}