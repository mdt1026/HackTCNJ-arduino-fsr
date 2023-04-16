import { type NextPage } from "next";
import { useState, useEffect } from "react";
import Head from "next/head";

import MainScene from "~/components/MainScene";
import Modal from "~/components/Modal";

const Home: NextPage = () => {
  const [toggleModal, setToggleModal] = useState(false);
  const [pads, setPads] = useState([]);

  useEffect(() => {
    console.log(pads);
  }, [pads]);

  return (
    <>
      <Head>
        <title>FSR Web Ui</title>
        <meta name="description" content="HackTCNJ Entry" />
        <link rel="icon" href="/favicon.ico" />
      </Head>

      <main className="flex h-full w-full items-center">
        <MainScene
          toggleModal={toggleModal}
          setToggleModalCallback={setToggleModal}
          pads={pads}
          setPadsCallback={setPads}
        />
        {toggleModal ? (
          <Modal pads={pads} setToggleModalCallback={setToggleModal} />
        ) : (
          <></>
        )}
      </main>
    </>
  );
};

export default Home;
