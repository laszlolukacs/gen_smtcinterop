#include "taglib_media_info_provider.h"

#include <algorithm>
#include <vector>

#include <taglib/apefile.h>
#include <taglib/asffile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacpicture.h>
#include <taglib/mpcfile.h>
#include <taglib/mpegfile.h>

std::vector<unsigned char> TaglibMediaInfoProvider::convert_to_uchar(const char* char_array, const unsigned int size) {
	std::vector<char> signed_vector(char_array, char_array + size);
	std::vector<unsigned char> unsigned_vector = std::vector<unsigned char>();
	std::copy(signed_vector.begin(), signed_vector.end(), std::back_inserter(unsigned_vector));
	return unsigned_vector;
}

AlbumArt TaglibMediaInfoProvider::extract_album_art_from_id3v2(TagLib::ID3v2::Tag* tag)
{
	const TagLib::ID3v2::FrameList& frame_list = tag->frameList("APIC");
	if (!frame_list.isEmpty())
	{
		const auto* frame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame_list.front());
		return AlbumArt(
			convert_to_uchar(frame->picture().data(), frame->picture().size()),
			std::wstring(frame->mimeType().toCWString()));
	}

	return AlbumArt();
}

AlbumArt TaglibMediaInfoProvider::extract_album_art_from_apev2(TagLib::APE::Tag* tag)
{
	const std::string album_cover_art_key = "COVER ART (FRONT)";
	const TagLib::APE::ItemListMap& ape_item_list_map = tag->itemListMap();
	if (ape_item_list_map.contains(album_cover_art_key))
	{
		const TagLib::ByteVector nullStringTerminator(1, 0);
		TagLib::ByteVector item = ape_item_list_map[album_cover_art_key].value();
		const int pos = item.find(nullStringTerminator); // skips the filename
		if (pos != -1)
		{
			const TagLib::ByteVector& picture = item.mid(pos + 1);
			return AlbumArt(
				convert_to_uchar(picture.data(), picture.size()),
				L"");
		}
	}

	return AlbumArt();
}

AlbumArt TaglibMediaInfoProvider::extract_album_art_from_flac(TagLib::FLAC::File* file)
{
	const TagLib::List<TagLib::FLAC::Picture*>& flac_picture_list = file->pictureList();
	if (!flac_picture_list.isEmpty())
	{
		const TagLib::FLAC::Picture* picture = flac_picture_list[0];
		return AlbumArt(
			convert_to_uchar(picture->data().data(), picture->data().size()),
			std::wstring(picture->mimeType().toCWString()));
	}

	return AlbumArt();
}

AlbumArt TaglibMediaInfoProvider::extract_album_art_from_mp4(TagLib::MP4::File* file)
{
	const std::string album_cover_art_key = "covr";
	TagLib::MP4::Tag* tag = file->tag();
	if (tag->item(album_cover_art_key).isValid())
	{
		const TagLib::MP4::CoverArtList& cover_art_list = tag->item(album_cover_art_key).toCoverArtList();
		if (!cover_art_list.isEmpty())
		{
			const TagLib::MP4::CoverArt* cover_art = &(cover_art_list.front());
			return AlbumArt(
				convert_to_uchar(cover_art->data().data(), cover_art->data().size()),
				L"");
		}
	}

	return AlbumArt();
}

AlbumArt TaglibMediaInfoProvider::extract_album_art_from_asf(TagLib::ASF::File* file)
{
	const std::string album_cover_art_key = "WM/Picture";
	const TagLib::ASF::AttributeListMap& attrListMap = file->tag()->attributeListMap();
	if (attrListMap.contains(album_cover_art_key))
	{
		const TagLib::ASF::AttributeList& asf_attribute_list = attrListMap[album_cover_art_key];
		if (!asf_attribute_list.isEmpty())
		{
			const TagLib::ASF::Picture& asf_picture = asf_attribute_list[0].toPicture();
			if (asf_picture.isValid())
			{
				return AlbumArt(
					convert_to_uchar(asf_picture.picture().data(), asf_picture.picture().size()),
					std::wstring(asf_picture.mimeType().toCWString()));
			}
		}
	}

	return AlbumArt();
}

AlbumArt TaglibMediaInfoProvider::extract_album_art(const TagLib::FileRef& fr)
{
	AlbumArt album_art;
	if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fr.file()))
	{
		if (file->ID3v2Tag())
		{
			album_art = extract_album_art_from_id3v2(file->ID3v2Tag());
		}
		if (album_art.empty() && file->APETag())
		{
			album_art = extract_album_art_from_apev2(file->APETag());
		}
	}
	else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fr.file()))
	{
		album_art = extract_album_art_from_flac(file);

		if (album_art.empty() && file->ID3v2Tag())
		{
			album_art = extract_album_art_from_id3v2(file->ID3v2Tag());
		}
	}
	else if (TagLib::MP4::File* file = dynamic_cast<TagLib::MP4::File*>(fr.file()))
	{
		album_art = extract_album_art_from_mp4(file);
	}
	else if (TagLib::ASF::File* file = dynamic_cast<TagLib::ASF::File*>(fr.file()))
	{
		album_art = extract_album_art_from_asf(file);
	}
	else if (TagLib::APE::File* file = dynamic_cast<TagLib::APE::File*>(fr.file()))
	{
		if (file->APETag())
		{
			album_art = extract_album_art_from_apev2(file->APETag());
		}
	}
	else if (TagLib::MPC::File* file = dynamic_cast<TagLib::MPC::File*>(fr.file()))
	{
		if (file->APETag())
		{
			album_art = extract_album_art_from_apev2(file->APETag());
		}
	}

	return album_art;
}

MediaInfo TaglibMediaInfoProvider::get_metadata_of_song(std::wstring filename)
{
	if (!filename.empty()) {
		const TagLib::FileRef f(filename.c_str());
		if (f.tag() != nullptr && !f.tag()->isEmpty())
		{
			TagLib::String artist = f.tag()->artist();
			TagLib::String title = f.tag()->title();
			AlbumArt album_art = extract_album_art(f);
			return MediaInfo(artist.toWString(), title.toWString(), album_art);
		}
	}

	return MediaInfo(L"", L"");
}
