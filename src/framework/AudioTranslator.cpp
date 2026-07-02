#include "AudioTranslator.h"

#include <QDebug>
#include <QThread>
#include <QVector>
#include <QString>
#include <QDataStream>
#include <QRegularExpression>

AudioTranslatorMgr *AudioTranslatorMgr::Instance()
{
    static AudioTranslatorMgr instance;
    return &instance;
}

AudioTranslatorMgr::AudioTranslatorMgr(QObject *parent)
    : QObject(parent)
    , m_translators()
{
}

AudioTranslatorMgr::~AudioTranslatorMgr()
{
    for(auto trans : m_translators.values())
    {
        if(trans->workerThread())
        {
            trans->workerThread()->quit();
            trans->workerThread()->wait();
        }
    }
    m_translators.clear();
}

AudioTranslator *AudioTranslatorMgr::Create(const QString &id,
                                            const QString &modelPath,
                                            const hj::asr::ctx_params_t &params)
{
    return Create(id,
                  modelPath,
                  params,
                  hj::asr::context::default_full_params());
}

AudioTranslator *
AudioTranslatorMgr::Create(const QString                &id,
                           const QString                &modelPath,
                           const hj::asr::ctx_params_t  &params,
                           const hj::asr::full_params_t &fullParams)
{
    if(m_translators.find(id) != m_translators.end())
        return nullptr;

    auto trans = new AudioTranslator(modelPath, params, fullParams, nullptr);
    auto workerThread = new QThread(this);
    trans->setWorkerThread(workerThread);
    trans->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, trans, &QObject::deleteLater);
    connect(this,
            &AudioTranslatorMgr::SignalProcessAudio,
            trans,
            &AudioTranslator::SlotProcessAudio);
    connect(trans,
            &AudioTranslator::SignalTranslationFinished,
            this,
            &AudioTranslatorMgr::SignalAudioTranslated);

    workerThread->start();
    m_translators[id] = trans;
    return trans;
}

AudioTranslator *AudioTranslatorMgr::Get(const QString &id)
{
    if(m_translators.isEmpty())
        return nullptr;

    if(id.isEmpty())
        return m_translators.values().at(0);
    else
        return m_translators[id];
}

int AudioTranslatorMgr::Translate(const QByteArray &src, const QString &id)
{
    auto trans = Get(id);
    if(trans == nullptr)
        return -1;

    emit SignalProcessAudio(src, trans->fullParams());
    return 0;
}

int AudioTranslatorMgr::Translate(const QByteArray            &src,
                                  const QString               &id,
                                  const hj::asr::full_params_t params)
{
    auto trans = Get(id);
    if(trans == nullptr)
        return -1;

    emit SignalProcessAudio(src, hj::asr::context::default_full_params());
    return 0;
}

void AudioTranslatorMgr::SlotAudioTranslate(const QByteArray &src,
                                            const QString    &id)
{
    Translate(src, id);
}


// ------------------------ translator -------------------------------
AudioTranslator::AudioTranslator(const QString               &modelPath,
                                 const hj::asr::ctx_params_t &params,
                                 QObject                     *parent)
    : QObject(parent)
    , m_ctx(modelPath.toStdString(), params)
    , m_fullParams{hj::asr::context::default_full_params()}
{
}

AudioTranslator::AudioTranslator(const QString                &modelPath,
                                 const hj::asr::ctx_params_t  &params,
                                 const hj::asr::full_params_t &fullParams,
                                 QObject                      *parent)
    : QObject(parent)
    , m_ctx(modelPath.toStdString(), params)
    , m_fullParams{fullParams}
{
}

AudioTranslator::~AudioTranslator()
{
}

void AudioTranslator::convert(std::vector<float> &pcmf32,
                              const QByteArray   &data)
{
    int sampleCount = data.size() / sizeof(int16_t);
    pcmf32.reserve(sampleCount);

    const int16_t *rawSamples =
        reinterpret_cast<const int16_t *>(data.constData());
    for(int i = 0; i < sampleCount; ++i)
    {
        float samplef = static_cast<float>(rawSamples[i]) / 32768.0f;

        if(samplef < -1.0f)
            samplef = -1.0f;

        if(samplef > 1.0f)
            samplef = 1.0f;

        pcmf32.push_back(samplef);
    }
}

hj::asr::full_params_t AudioTranslator::fullParams()
{
    return m_fullParams;
}

void AudioTranslator::setMuteAmplitudeDurationMs(int durMs)
{
    m_muteAmplitudeDurationMs = durMs;
}

void AudioTranslator::setMuteAmplitudeThreshold(float threshold)
{
    m_muteAmplitudeThreshold = threshold;
}

void AudioTranslator::setMinAudioBufferSize(int size)
{
    m_minAudioBufferSize = size;
}

void AudioTranslator::setMinNewSampleSize(int size)
{
    m_minNewSampleSize = size;
}

void AudioTranslator::setKeepLastAudioBufferMs(int ms)
{
    m_keepLastAudioBufferMs = ms;
}

bool AudioTranslator::checkCurrSegmentFinished(const QString &catched)
{
    if(m_muteCount < 3)
        return false;

    if(!catched.isEmpty())
        return true;

    if(m_muteCount >= 5)
        return true;

    return false;
}

void AudioTranslator::checkAmplitude()
{
    auto lastMs             = muteAmplitudeDurationMs();
    auto threshold          = muteAmplitudeThreshold();
    auto needToCheckSamples = static_cast<size_t>(16000 * lastMs / 1000);
    if(m_pcmBuf.size() < needToCheckSamples) // not enough samples to check
        return;

    auto  check_samples = qMin(m_pcmBuf.size(), needToCheckSamples);
    float maxAmplitude  = 0.0;
    for(size_t i = m_pcmBuf.size() - check_samples; i < m_pcmBuf.size(); ++i)
        maxAmplitude = qMax(maxAmplitude, qAbs(m_pcmBuf[i]));

    qDebug() << "checkAmplitude - max:" << maxAmplitude
             << "threshold:" << threshold << "muteCount:" << m_muteCount;

    if(maxAmplitude < threshold)
        m_muteCount++;
    else
        m_muteCount = 0;
}

bool AudioTranslator::checkBufSize()
{
    return m_pcmBuf.size() > minAudioBufferSize();
}

bool AudioTranslator::checkNewSampleSize(int newSampleSize)
{
    m_newSampleSize += newSampleSize;
    return m_newSampleSize > minNewSampleSize();
}

void AudioTranslator::filt(QString &str)
{
    QRegularExpression tagRegex(m_filtRegex);
    str.remove(tagRegex);

    // remove noise words
    for(const auto &noise : m_noiseWords)
    {
        qDebug() << "filt noise word:" << noise;
        if(!noise.isEmpty())
            str.replace(noise, "", Qt::CaseInsensitive);
    }

    str.replace(QRegularExpression("\\s+"), " ");
    str = str.trimmed();
}

int AudioTranslator::translate(QVector<QString> &segments,
                               const QByteArray &data)
{
    return translate(segments, data, m_fullParams);
}

int AudioTranslator::translate(QVector<QString>             &segments,
                               const QByteArray             &data,
                               const hj::asr::full_params_t &params)
{
    // translate audio data to pcmf32
    std::vector<float> pcmf32;
    convert(pcmf32, data);
    m_pcmBuf.reserve(m_pcmBuf.size() + pcmf32.size());
    m_pcmBuf.insert(m_pcmBuf.end(), pcmf32.begin(), pcmf32.end());

    // check buffer size
    if(!checkBufSize())
        return 0;

    // if new sample too small, return
    if(!checkNewSampleSize(pcmf32.size()))
        return 0;

    // check the last n ms amplitue
    checkAmplitude();

    // translate pcmf32 to text
    auto err = m_ctx.full(params, m_pcmBuf);
    qDebug() << "m_pcmBuf.size():" << m_pcmBuf.size();
    if(err != 0)
        return err;

    // parse segments
    QString tmpStr;
    auto    n_segments = m_ctx.n_segments();
    qDebug() << "n_segments:" << n_segments;
    for(auto i = 0; i < n_segments; ++i)
    {
        std::string segment;
        m_ctx.get_segment_text(segment, i);
        tmpStr += QString::fromStdString(segment);
        qDebug() << "parse segment:" << QString::fromStdString(segment);
    }

    // filt noise
    filt(tmpStr);
    qDebug() << "After filt - tmpStr:" << tmpStr
             << ", muteCount:" << m_muteCount
             << ", buffer size:" << m_pcmBuf.size();
    if(tmpStr.isEmpty())
        return 0;

    // check finished or not
    bool isFinished = checkCurrSegmentFinished(tmpStr);
    qDebug() << "checkCurrSegmentFinished result:" << isFinished;
    if(isFinished)
    {
        segments.append(tmpStr);

        // clear buffer, keep the last 1 second
        size_t keepSize = (size_t) 16000 * keepLastAudioBufferMs() / 1000;
        keepSize        = qMin(m_pcmBuf.size(), keepSize);
        std::vector<float> keepBuf(m_pcmBuf.end() - keepSize, m_pcmBuf.end());
        m_pcmBuf = keepBuf;

        m_muteCount     = 0;
        m_newSampleSize = 0;
    }

    return 0;
}

void AudioTranslator::SlotProcessAudio(const QByteArray             &data,
                                       const hj::asr::full_params_t &params)
{
    QVector<QString> segments;
    int              ec = translate(segments, data, params);
    if(ec == 0 && !segments.isEmpty())
    {
        emit SignalTranslationFinished(ec, data, segments);
    }
}